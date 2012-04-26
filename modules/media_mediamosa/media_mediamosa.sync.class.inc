<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
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
 * Static class for sync related functions.
 */

class MediaMediaMosaSync {
/**
   * Syncs a set of assets with our database.
   *
   * @param $offset
   *   The offset of the range.
   * @param $limit
   *   The limit of the range.
   *
   * @return
   *   An array filled with data about the sync.
   */
  static public function sync_mediamosa_to_drupal_by_range($offset, $limit) {
    $assets_created = 0;
    $assets_updated = 0;

    // Get the assets.
    $assets = MediaMosaConnectorMediaWrapper::get_current_assets(0, $offset, $limit);

    // Now loop through the found assets.
    foreach ($assets as $asset) {
      $asset_id = $asset['asset_id'];
      $media_id = _media_mediamosa_get_media_id($asset_id);

      // Not found in our database? Then create it.
      if (!$media_id) {
        MediaMediaMosa::create_media($asset);
        $assets_created++;
      }
      else {
        // Found, update it.
        MediaMediaMosa::update_media($asset, $media_id);
        $assets_updated++;
      }
    }

    return array(
      'offset' => $offset + $limit, // Return next offset.
      'processed_total' => count($assets),
      'assets_created' => $assets_created,
      'assets_updated' => $assets_updated
    );
  }

  /**
   * Syncs a range of file_managed and compares it MediaMosa. Removes any
   * missing Media that is no longer found in MediaMosa.
   *
   * @param $offset
   *   The offset of the range.
   * @param $limit
   *   The limit of the range.
   *
   * @return
   *   An array filled with data about the sync.
   */
  static public function sync_drupal_to_mediamosa_by_range($offset, $limit) {

    // Do a query.
    $query = db_select('file_managed', 'fm');
    $query->fields('fm', array('fid', 'uri'));
    $query->condition('uri', 'mediamosa://%', 'LIKE');
    $query->orderBy('fid', 'ASC');
    $query->range($offset, $limit);
    $result = $query->execute()->fetchAll();

    // No media found? Then reset offset to 0 and none where deleted.
    if (!count($result)) {
      return array('offset' => 0, 'deleted' => 0, 'processed_total' => 0);
    }
    // Set next pos.
    $offset += $limit;

    $assetids_to_fids = array();
    foreach ($result as $media) {
      $uri_parts = MediaMosaStreamWrapper::mediamosa_parse_url($media->uri);
      $assetids_to_fids[$uri_parts['asset_id']] = $media->fid;
    }

    // Get the asset_ids.
    $asset_ids = array_keys($assetids_to_fids);

    // Get the assets that do exists at side of MediaMosa.
    $asset_ids_exists = MediaMosaConnectorMediaWrapper::request_check_assets_existense($asset_ids);

    // Find out which assets are missing.
    $asset_ids_missing = array_diff($asset_ids, $asset_ids_exists);

    // So we know how many assets we processed.
    $processed_total = count($asset_ids);

    // Ok now loop and remove them from frontend side.
    foreach ($asset_ids_missing as $asset_id) {
      $fid = $assetids_to_fids[$asset_id];

      // Delete it from Drupal.
      $file = file_load($fid);
      file_delete($file, TRUE);
    }

    // Calculate the next position in file_managed. Remove the number of deleted
    // for the correct next position so we don't skip any.
    $offset += ($limit - count($asset_ids_missing));

    // Return next offset.
    return array('offset' => $offset, 'deleted' => count($asset_ids_missing), 'processed_total' => $processed_total);
  }

  /**
   * When asset has been deleted on the mediamosa side, we have to remove
   * corresponding Media entity.
   */
  static public function cron_sync_drupal_to_mediamosa() {

    // Dont set this to high, don't choke the cron.
    // Set the # of runs in this cron and per run # of media to check.
    $runs = 3;
    $limit = 10; // Keep this below 30.

    // Is sync from Drupal to MediaMosa on for cron?
    if (!self::do_sync_drupal_to_mediamosa()) {
      MediaMediaMosa::watchdog('Sync from Drupal to MediaMosa is off', array(), WATCHDOG_INFO);
      return;
    }

    // Get exclusive lock.
    if (!lock_acquire(__FUNCTION__)) {
      return;
    }

    // Connector not setup.
    if (!MediaMosaConnectorMediaWrapper::check_connection()) {
      MediaMediaMosa::watchdog('Unanle to connect with MediaMosa; can not sync Drupal to MediaMosa');
      return;
    }

    MediaMediaMosa::watchdog('Drupal->MediaMosa sync run started', array(), WATCHDOG_DEBUG);

    // Get the current position.
    $offset = variable_get('mediamosa_ck_pos_sync_drupal_to_mediamosa', 0);

    $processed_total = 0;
    $deleted = 0;

    while ($runs--) {
      $result = MediaMediaMosaSync::sync_drupal_to_mediamosa_by_range($offset, $limit);

      // Calculated total.
      $deleted += $result['deleted'];

      // Next position.
      $offset = $result['offset'];

      // Total processed.
      $processed_total += $result['processed_total'];

      // As soon offset is reset, we are done.
      if (!$offset) {
        break;
      }
    }

    // Save next position for next run.
    variable_set('mediamosa_ck_pos_sync_drupal_to_mediamosa', $offset);

    // Release lock.
    lock_release(__FUNCTION__);

    // Log our result.
    MediaMediaMosa::watchdog('Drupal->MediaMosa sync run finished, @deleted of the @processed_total processed assets are abandoned. Removed @deleted media from website, which where no longer found in MediaMosa.', array('@processed_total' => $processed_total, '@deleted' => $deleted), WATCHDOG_DEBUG);
  }

  /**
   * Do the syncronization MediaMosa -> Drupal.
   */
  static public function cron_sync_mediamosa_to_drupal() {

    // Is sync from MediaMosa to Drupal cron on?
    if (!self::do_sync_mediamosa_to_drupal()) {
      MediaMediaMosa::watchdog('Sync from MediaMosa to Drupal is off', array(), WATCHDOG_INFO);
      return;
    }

    // Get exclusive lock.
    if (!lock_acquire(__FUNCTION__)) {
      return;
    }

    // Connector not setup, don't bother.
    if (!MediaMosaConnectorMediaWrapper::check_connection()) {
      MediaMediaMosa::watchdog('Unanle to connect with MediaMosa; can not sync with MediaMosa');
      return;
    }

    $time_last = variable_get('media_mediamosa_sync_time_' . MediaMosaConnectorMediaWrapper::build_connection_id(), 0);
    $offset_last = variable_get('media_mediamosa_sync_offset_' . MediaMosaConnectorMediaWrapper::build_connection_id(), 0);

    MediaMediaMosa::watchdog('Sync from MediaMosa to Drupal started, highest change/modified time @time_last.', array('@time_last' => $time_last ? format_date($time_last) : t('never')), WATCHDOG_DEBUG);

    // These two introduced just for statistic purposes.
    $assets_updated = 0;
    $assets_created = 0;

    // Really low what we are going to do here. 1st sync will take time. Lets not
    // choke cron.
    $limit = max(min(variable_get('media_mediamosa_cron_sync_throttle_cron_limit', 100), 199), 10);

    // Get the assets.
    $assets = MediaMosaConnectorMediaWrapper::get_current_assets($time_last, $offset_last, $limit + 1);

    if (!empty($assets)) {
      // Get the first asset.
      $asset_first = reset($assets);
      // Get the last asset.
      $asset_last = count($assets) == $limit + 1 ? array_pop($assets) : FALSE;

      $videotimestampmodified = $asset_first['videotimestampmodified'];

      // Old offset, so we continue there.
      $offset = $offset_last - 1;
      foreach ($assets as $asset) {
        $asset_id = $asset['asset_id'];
        $media_id = _media_mediamosa_get_media_id($asset_id);


        // Not found in our database? Then create it.
        if (!$media_id) {
          MediaMediaMosa::create_media($asset);
          $assets_created++;
        }
        else {
          // Found, update it.
          MediaMediaMosa::update_media($asset, $media_id);
          $assets_updated++;
        }

        // Next offset.
        $offset++;

        // Everytime the modified of the previous is not the same, reset the
        // offset. The $offset is the offset within the result of that
        // videotimestampmodified.
        if ($videotimestampmodified != $asset['videotimestampmodified']) {
          $offset = 0;
        }

        // Save the last videotimestampmodified.
        $videotimestampmodified = $asset['videotimestampmodified'];
      }

      // More to come? ($asset_last is only set when we had more results than
      // limit).
      //
      // Has the last asset (that we didn't process) and the last processed asset
      // have the same videotimestampmodified? Then take the offset when we first
      // offset found with this videotimestamp and store it, so the next time the
      // videotimestampmodified is used again but with the next unsynced results.
      if ($asset_last && $asset_last['videotimestampmodified'] == $videotimestampmodified) {
        $time_last = mediamosa_ck::date2unix($videotimestampmodified);
        variable_set('media_mediamosa_sync_time_' . MediaMosaConnectorMediaWrapper::build_connection_id(), $time_last);
        variable_set('media_mediamosa_sync_offset_' . MediaMosaConnectorMediaWrapper::build_connection_id(), $offset + 1);
      }
      else {
        // Next time we'll take everything that is new or changed.
        $time_last = mediamosa_ck::date2unix($videotimestampmodified) + 1;
        variable_set('media_mediamosa_sync_time_' . MediaMosaConnectorMediaWrapper::build_connection_id(), $time_last);
        variable_set('media_mediamosa_sync_offset_' . MediaMosaConnectorMediaWrapper::build_connection_id(), 0);
      }
    }

    MediaMediaMosa::watchdog(
      'MediaMosa->Drupal sync run finished, There where @new_assets new assets of total @total_assets assets. Highest change/modified set to @time_last.',
      array(
        '@new_assets' => $assets_created,
        '@total_assets' => $assets_created + $assets_updated,
        '@time_last' => format_date($time_last),
      ),
      WATCHDOG_INFO
    );

    // Release lock.
    lock_release(__FUNCTION__);

    // Useful when we run this via GUI.
    return array(
      'processed_total' => $assets_created + $assets_updated,
      'assets_created' => $assets_created,
      'assets_updated' => $assets_updated,
    );
  }

  /**
   * Return the setting for during the cron sync MediaMosa to Drupal.
   *
   * @return
   *   TRUE or FALSE.
   */
  static public function do_cron_sync_mediamosa_to_drupal() {
    return variable_get('media_mediamosa_cron_sync_mediamosa_to_drupal', TRUE);
  }

  /**
   * Return the setting for during the cron sync MediaMosa to Drupal.
   *
   * @return
   *   TRUE or FALSE.
   */
  static public function do_cron_sync_drupal_to_mediamosa() {
    return variable_get('media_mediamosa_cron_sync_drupal_to_mediamosa', FALSE);
  }

  /**
   * Return the setting for syncing Drupal to MediaMosa during the sync batch.
   *
   * @return
   *   TRUE or FALSE.
   */
  static public function do_sync_drupal_to_mediamosa() {
    return variable_get('media_mediamosa_sync_drupal_to_mediamosa', FALSE);
  }

  /**
   * Return the setting for syncing Drupal to MediaMosa during the sync batch.
   *
   * @return
   *   TRUE or FALSE.
   */
  static public function do_sync_mediamosa_to_drupal() {
    return variable_get('media_mediamosa_sync_mediamosa_to_drupal', TRUE);
  }
}
