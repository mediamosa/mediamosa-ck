<?php
/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View will return the featured assets.
 *
 */
class mediamosa_ck_views_rest_asset_get extends mediamosa_ck_views_rest {

  /**
   * Execute the REST call.
   *
   * @param $view
   *   The view object.
   * @param $params
   *   Array of options and settings.
   */
  protected function do_rest_call($view, $params, array $options = array()) {
    // Total results.
    $view->total_rows = 0;

    // Get the asset_id.
    $asset_id = empty($view->args) ? '' : $view->args[0];

    if (empty($asset_id)) {
      return;
    }

    // Specific for playing.
    $mediafile_id = empty($view->args[1]) ? '' : $view->args[1];

    $options += array(
      'show_stills' => TRUE,
      'show_collections' => TRUE,
    );

    try {
      // Get the asset.
      $xml = MediaMosaCkConnectorWrapper::get_asset($asset_id, $options);
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve asset from MediaMosa, message: @message.', array('@message' => $e->getMessage()));
      return;
    }

    $fields = array(
      'viewed', 'played',
      'videotimestamp', 'videotimestampmodified',
      'owner_id', 'group_id',
    );

    // Now process the xml into an object views can work with.
    foreach($xml->xpath('items/item') as $item) {
      $object = new stdClass();
      $object->asset_id = (string) $item->asset_id;
      $object->granted = drupal_strtoupper((string) $item->granted) == 'TRUE' ? 'TRUE' : 'FALSE';
      $object->is_owner = mediamosa_ck::session_user_id() == $item->owner_id;

      // Asset Metadata.
      foreach (mediamosa_ck::get_metadata_fields() as $set => $metadata) {
        foreach ($item->xpath($metadata['xpath']) as $metadata_value) {
          foreach ($metadata_value as $name => $value) {
            // In variable.
            $object->{$set . '.' . (string) $name} = (string) $value;

            // In set.
            $object->{'metadata_' . $set}[(string) $name] = (string) $value;
          }
        }
      }

      // Copy these values.
      foreach ($fields as $field) {
        // @todo need to get default value when value is empty.
        $object->{$field} = isset($item->{$field}) ? (string) $item->{$field} : '';
      }

      // Collection data.
      foreach ($item->xpath('collections/collection') as $collection) {
        $object->collections[(string) $collection->attributes()->id] = array('title' => (string) $collection->title, 'coll_id' => (string) $collection->attributes()->id);
      }

      // Empty by default.
      $object->technical_metadata = array();

      // Mediafiles.
      foreach ($item->xpath('mediafiles/mediafile') as $mediafile) {
        $mediafile = self::xmlobject2array($mediafile);
        $object->mediafiles[(string) $mediafile['mediafile_id']] = $mediafile;

        // Find original and copy its technical metadata.
        if (drupal_strtoupper($mediafile['is_original_file']) == 'TRUE' ) {
          $object->technical_metadata = $mediafile['metadata'];
          $object->mediafile_id_original = $mediafile['mediafile_id'];
        }
      }

      // The name of the field to use.
      $title_name = mediamosa_ck::get_metadata_title();

      // @todo need to get default value when title is empty.
      $object->title = $object->{$title_name};

      $view->result[] = $object;
    }

    // Assign total number of records to get pager.
    $view->total_rows = (int) $xml->header->item_count_total;
  }

  /**
   * Get the views data for this REST call.
   *
   * @return
   */
  static public function get_views_data($title, $class) {
    $data = array(
      'table' => array(
        'group' => t('MediaMosa'),
        'base' => array(
          'field' => 'id',
          'title' => t($title),
          'help' => t('Retrieve a asset using the MediaMosa backend'),
          'query class' => $class,
        ),
      ),
    );

    // Add asset stuff.
    self::get_views_data_asset_common($data);

    // Add asset specific.
    self::get_views_data_asset_specific($data);

    // Owner / group.
    self::get_views_data_owner_stuff($data);

    // Make asset_id contextual.
    $data['asset_id']['argument'] = array(
      'handler' => 'views_handler_argument_string',
    );

    // Add metadata.
    self::get_views_data_metadata($data);

    // Mediafiles.
    $data['mediafiles'] = array(
      'title' => t('Mediafiles'),
      'help' => t('Shows a listing of the mediafiles.'),
      'field' => array(
        'handler' => 'mediamosa_ck_views_field_text_mediafiles',
      ),
    );

    // Collections.
    $data['collections'] = array(
      'title' => t('Collections'),
      'help' => t('Shows a listing of the collection.'),
      'field' => array(
        'handler' => 'mediamosa_ck_views_field_text_collections',
      ),
    );

    // Collections.
    $data['technical_metadata'] = array(
      'title' => t('Technical metadata'),
      'help' => t('Show the technical metadata of the asset.'),
      'field' => array(
        'handler' => 'mediamosa_ck_views_field_text_technical_metadata',
      ),
    );

    // Metadata blocks.
    foreach (mediamosa_ck::get_metadata_fields() as $set => $metadata) {
      // Mediafiles.
      $data['metadata_' . $set] = array(
        'title' => t('Metadata block for ' . $metadata['title']),
        'help' => t('Shows a listing of the metadata.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text_metadata',
        ),
      );
    }


    return $data;
  }
}
