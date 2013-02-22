<?php
/**
 * @file
 * Static class for helper functions.
 */

class MediaMediaMosa {

  // Settings;
  // Expire default for thumbnails in seconds.
  const THUMBNAIL_EXPIRE_TIME = 600;

  /**
   * Wrapper for quicker logging.
   */
  public static function watchdog($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL, $type = '') {
    $type = 'Media MediaMosa' . (empty($type) ? '' : ' - ' . $type);
    watchdog(substr($type, 0, 64), $message, $variables, $severity, $link);
  }

  /**
   * Check and validate a datestamp string for metadata.
   *
   * returns FALSE or valid string to use.
   *
   * @param string $str_date
   *   string to check.
   *
   * @return string
   *   or FALSE
   */
  public static function check_datestamp($str_date) {

    $str_date = trim($str_date);
    if ($str_date == '') {
      // Empty is ok.
      return '0000-00-00 00:00:00';
    }

    if (!preg_match('/^\d{4}-\d{2}-\d{2}( \d{2}:\d{2}:\d{2})?$/', $str_date)) {
      return FALSE;
    }

    if (strlen($str_date) == 10) {
      $str_date .= ' 00:00:00';
    }

    list($year, $month, $day, $hour, $minute, $second) = preg_split('/[:\- ]{1}/', $str_date);

    if (!checkdate($month, $day, $year)) {
      return FALSE;
    }

    if ($hour > 23 || $minute > 59 || $second > 59) {
      return FALSE;
    }

    return $str_date;
  }

  /**
   * Create Drupal media object in accordance with given asset info.
   */
  public static function create_media($asset) {
    try {
      $mediafiles = MediaMosaConnectorMediaWrapper::get_asset_mediafiles($asset['asset_id']);

      $mediafile = FALSE;
      foreach ($mediafiles as $mediafile) {
        if ($mediafile['is_original_file'] == 'TRUE') {
          break;
        }
      }

      if ($mediafile && $mediafile['is_original_file'] == 'TRUE') {
        // Build the scheme uri (mediamosa://asset/[id]/[id]/[filename]).
        $uri = strtr('mediamosa://asset/@asset_id/@mediafile_id/@filename', array(
                 '@asset_id' => $mediafile['asset_id'],
                 '@mediafile_id' => $mediafile['mediafile_id'],
                 '@filename' => $mediafile['filename'],
               )
        );

        // Save it using uri.
        $file = file_uri_to_object($uri);
        file_save($file);
      }
    }
    catch (Exception $e) {
      MediaMediaMosa::watchdog('MediaMediaMosa::create_media(): cannot create file for URI @uri: @error.', array('@uri' => $uri, '@error' => $e->getMessage()), WATCHDOG_ERROR);
      return FALSE;
    }

    MediaMediaMosa::watchdog('Created media object @media_id to reflect URI @uri.', array('@uri' => $uri, '@media_id' => $file->fid), WATCHDOG_DEBUG);

    // Call update right now to populate Dublin Core and technical metadata.
    MediaMediaMosa::update_media($asset, $file->fid);

    return $file->fid;
  }

  /**
   * Update Drupal media object by given asset info.
   *
   * @param array() $asset
   *   asset info.
   * @param int $media_id
   *   media id.
   */
  public static function update_media($asset, $media_id) {

    // Processing the Dublin core metadata fields.
    $metadata = array_merge($asset['dublin_core'], $asset['qualified_dublin_core']);

    // To distinct data in Drupal and in Mediamosa we use drupal_ prefix for
    // the Drupal side variables.
    $drupal_multiple_media = media_multi_load("{$media_id}");
    $drupal_media = $drupal_multiple_media[$media_id];

    $drupal_uri = MediaMosaStreamWrapper::mediamosa_parse_url($drupal_media->uri);
    if (empty($drupal_uri)) {
      return;
    }

    $drupal_mediafile = $drupal_uri['mediafile_id'];
    $changes_counter = 0;

    foreach ($metadata as $key => $value) {
      $field_name = FIELD_PREFIX . $key;
      $drupal_ref = &$drupal_file->$field_name;

      $drupal_value = empty($drupal_ref) ? '' : $drupal_ref[LANGUAGE_NONE]['0']['value'];

      if ($drupal_value != $value) {
        $drupal_ref[LANGUAGE_NONE]['0']['value'] = is_array($value) ? $value[0] : $value;
        $changes_counter++;
      }
    }

    // We have done with DC/QDC fields. Now lets take care about mediafiles
    // itself and mediafiles tech info.
    $mediafiles = MediaMosaConnectorMediaWrapper::get_asset_mediafiles($asset['asset_id']);

    // We get array of arrays for each asset. But we need one global array for
    // all assets. So pick just first element of each. It doesn't seem like a
    // best solution.
    $mediafiles = reset($mediafiles);

    if (empty($mediafiles)) {
      MediaMediaMosa::watchdog('No mediafiles at all while looking for @mediafile, asset @asset_id.',
                 array('@mediafile' => $drupal_mediafile, '@asset_id' => $asset['asset_id']), WATCHDOG_ERROR);
      return FALSE;
    }

    $mediafile = FALSE;

    if ($mediafiles['mediafile_id'] == $drupal_mediafile) {
      // We found a match.
      $mediafile = $mediafiles;
    }

    if (empty($mediafile)) {
      // We didn't find our file, lets use other file from this asset.
      $mediafile = $mediafiles;
      MediaMediaMosa::watchdog('Mediafile @mediafile disappeared, will use another one from this asset @asset_id. Media id is @media_id.',
        array(
          '@mediafile' => $drupal_mediafile,
          '@asset_id' => $asset['asset_id'],
          '@media_id' => $media_id),
        WATCHDOG_NOTICE);
      $drupal_media->uri = 'mediamosa://asset/' . $asset['asset_id'] . '/' . $mediafile['mediafile_id'] . '/' . $mediafile['filename'];
      $changes_counter++;
    }

    $tech_metadata = $mediafile['metadata'];

    // The mediafile_id and is_original_file are special cases.  Mostly
    // parameters can be taken directly from 'metadata' subarray, keys are the
    // same.
    $field_name = FIELD_PREFIX . 'mediafile_id';
    if (empty($drupal_media->{$field_name}) || $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] != $mediafile['mediafile_id']) {
      $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] = $mediafile['mediafile_id'];
      $changes_counter++;
    }

    $field_name = FIELD_PREFIX . 'original';
    if (empty($drupal_media->{$field_name}) || $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] != $mediafile['is_original_file']) {
      $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] = $mediafile['is_original_file'];
      $changes_counter++;
    }

    // Video size is combined from two metavariables.
    $size = $tech_metadata['width'] . 'px * ' . $tech_metadata['height'] . 'px';
    $field_name = FIELD_PREFIX . 'size';
    if (empty($drupal_media->{$field_name}) || $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] != $size) {
      $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] = $size;
      $changes_counter++;
    }

    // And here goes transparent stuff.
    $meta_keys = array(
      'file_duration', 'filesize', 'mime_type', 'container_type', 'fps', 'video_codec',
    );

    foreach ($meta_keys as $key) {
      $field_name = FIELD_PREFIX . $key;
      if (empty($drupal_media->{$field_name}) || $drupal_media->{$field_name}[LANGUAGE_NONE][0]['value'] != $tech_metadata[$key]) {
        $drupal_media->{$field_name}[LANGUAGE_NONE][0]['value'] = $tech_metadata[$key];
        $changes_counter++;
      }
    }

    // Fill in image dimensions to prevent notice when updating.
    if ($drupal_media->type == 'image' && !isset($drupal_media->image_dimensions)) {
      $drupal_media->image_dimensions = '';
      $drupal_media->original->image_dimensions = '';
    }

    if ($changes_counter) {
      try {
        file_save($drupal_media);
      }
      catch (Exception $e) {
        MediaMediaMosa::watchdog('Error on update media for media @media_id, asset @asset_id: @error.',
          array(
            '@media_id' => $media_id,
            '@asset_id' => $asset['asset_id'],
            '@error' => $e->getMessage(),
          ),
          WATCHDOG_ERROR);
        return FALSE;
      }
      MediaMediaMosa::watchdog('Updated @counter properties for media object @media_id while syncronizing with Mediamosa.',
                 array('@counter' => $changes_counter, '@media_id' => $media_id), WATCHDOG_DEBUG);
    }
  }

  /**
   * Figure out how many assets are in MediaMosa.
   *
   * @return integer
   *   The number of assets in MediaMosa.
   */
  public static function assets_in_mediamosa() {
    return MediaMosaConnectorMediaWrapper::get_number_of_assets();
  }

  /**
   * Figure out how many assets are in file_managed.
   *
   * @return integer
   *   The number of assets in Drupal.
   */
  public static function assets_in_drupal() {
    $query = db_select('file_managed', 'fm');
    $query->condition('uri', 'mediamosa://%', 'LIKE');
    return $query->countQuery()->execute()->fetchField();
  }

  /**
   * Figure out the fid from an asset.
   *
   * @param string $asset_id
   *   The id of the asset
   *
   * @return integer
   *   The fid in Drupal.
   */
  static public function get_fid_from_asset($asset_id) {
    // Some cache.
    $assetids_to_fids = &drupal_static('get_fid_from_asset', array());

    if (isset($assetids_to_fids[$asset_id])) {
      return $assetids_to_fids[$asset_id];
    }

    // When to many, remove the 1st in the array.
    // Need to be careful with memory.
    if (count($assetids_to_fids) > 10000) {
      array_shift($assetids_to_fids);
    }

    $query = db_select('file_managed', 'fm');
    $query->fields('fm', array('fid'));
    $query->condition('uri', 'mediamosa://asset/' . db_like($asset_id) .  '/%', 'LIKE');
    $query->orderBy('fid', 'ASC');
    $assetids_to_fids[$asset_id] = $query->execute()->fetchField();

    return $assetids_to_fids[$asset_id];
  }
}
