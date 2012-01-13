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
 * View is based on the REST call 'asset'.
 *
 */
class mediamosa_ck_views_rest_asset_search extends mediamosa_ck_views_rest {

  /**
   * Execute the REST call.
   *
   * @param $view
   *   The view object.
   * @param $params
   *   Array of options and settings.
   */
  protected function do_rest_call($view, $params, array $options = array()) {
    $options += array(
      'offset' => $params['start'],
      'limit' => $params['rows'],
      'hide_empty_assets' => 'TRUE',
    );

    // Mediamosa only supports one param order, so take the first.
    $orderby = empty($this->orderby) ? FALSE : reset($this->orderby);
    if ($orderby) {
      $options['order_by'] = $orderby['orderby'];
      $options['order_direction'] = $orderby['order'] == 'ASC' ? 'asc' : 'desc';
    }

    // Do the search.
    $response = MediaMosaCkConnectorWrapper::search_asset($options);

    $fields = array(
      'viewed', 'played',
      'videotimestamp', 'videotimestampmodified',
      'owner_id', 'group_id',
      'mediafile_duration',
    );

    // Now process the xml into an object views can work with.
    foreach($response->xml->xpath('items/item') as $item) {
      $object = new stdClass();
      $object->asset_id = (string) $item->asset_id;
      $object->granted = drupal_strtoupper((string) $item->granted) == 'TRUE' ? 'TRUE' : 'FALSE';
      $object->is_owner = mediamosa_ck::session_user_id() == $item->owner_id;

      foreach (mediamosa_ck::get_metadata_fields() as $set => $metadata) {
        foreach ($item->xpath($metadata['xpath']) as $metadata_value) {
          foreach ($metadata_value as $name => $value) {
            $object->{$set . '.' . (string) $name} = (string) $value;
          }
        }
      }

      // Fill the array.
      foreach ($fields as $field) {
        // @todo need to get default value when value is empty.
        $object->{$field} = isset($item->{$field}) ? (string) $item->{$field} : '';
      }

      // The name of the field to use.
      $title_name = mediamosa_ck::get_metadata_title();

      // @todo need to get default value when title is empty.
      $object->title = $object->{$title_name};

      $view->result[] = $object;
    }

    // Assign total number of records to get pager.
    $view->total_rows = (int) $response->xml->header->item_count_total;
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
          'help' => t('Retrieve and search assets in the MediaMosa backend'),
          'query class' => $class,
        ),
      ),

      // Search fields.
      'keywords' => array(
        'title' => t('The search box'),
        'help' => t('Unique Identifier of each asset.'),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
      ),
    );

    // Add asset stuff.
    self::get_views_data_asset_common($data);

    // Owner / group.
    self::get_views_data_owner_stuff($data);

    // Add metadata.
    self::get_views_data_metadata($data);

    return $data;
  }

  /**
   * Is called when sort option is added/used.
   */
  public function add_orderby($table, $field = NULL, $order = 'ASC', $alias = '', $params = array()) {

    // Some fields have another orderby field...
    $translate = array(
      'viewed' => 'numofviews',
      'played' => 'numofplays',
    );

    // See if we have alias.
    if (isset($translate[$field])) {
      $field = $translate[$field];
    }

    parent::add_orderby($table, $field, $order, $alias, $params);
  }
}
