<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Syncronizing data between Mediamosa and Drupal. We have the same data in both systems.
 * Architecture is that Mediamosa is a 'master', Drupal is 'slave'.
 * This file provides synchronizing mechanisms, syncronization can be made by cron and by invoke via admin menu.
 *
 */

require_once (dirname(__FILE__) . '/media_mediamosa.inc');

/**
 * When asset has been deleted on the mediamosa side, we have to remove
 * corresponding Media entity.
 */
function _media_mediamosa_cron_sync_drupal_to_mediamosa() {

  // Dont set this to high, don't choke the cron.
  // Set the # of runs in this cron and per run # of media to check.
  $runs = 3;
  $limit = 10; // Keep this below 30.

  // Is sync from Drupal to MediaMosa on for cron?
  if (!variable_get('media_mediamosa_cron_sync_drupal_to_mediamosa', 0)) {
    return;
  }

  // Connector not setup.
  if (!MediaMosaConnectorMediaWrapper::check_connection()) {
    return;
  }

  mediamosa_ck::watchdog('Drupal->MediaMosa sync run started', array(), WATCHDOG_DEBUG);

  // Get the current position.
  $offset = variable_get('mediamosa_ck_pos_sync_drupal_to_mediamosa', 0);

  $processed_total = 0;
  $deleted = 0;

  while ($runs--) {
    $result = _media_mediamosa_sync_drupal_to_mediamosa_by_range($offset, $limit);

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

  // Log our result.
  mediamosa_ck::watchdog('Drupal->MediaMosa sync run finished, @deleted of the @processed_total processed assets are abandoned. Removed @deleted media from website, which where no longer found in MediaMosa.', array('@processed_total' => $processed_total, '@deleted' => $deleted), WATCHDOG_DEBUG);
}

/**
 * Syncs a range of file_managed and compares it MediaMosa. Removes any missing
 * Media that is no longer found in MediaMosa.
 */
function _media_mediamosa_sync_drupal_to_mediamosa_by_range($offset, $limit) {

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
 * Do the syncronization MediaMosa -> Drupal.
 */
function _media_mediamosa_cron_sync_mediamosa_to_drupal() {

  // Is sync from MediaMosa to Drupal cron on?
  if (!variable_get('media_mediamosa_cron_sync_mediamosa_to_drupal', 0)) {
    return;
  }

  // Connector not setup, don't bother.
  if (!MediaMosaConnectorMediaWrapper::check_connection()) {
    return;
  }

  $time_last = variable_get('media_mediamosa_sync_time_' . MediaMosaConnectorMediaWrapper::build_connection_id(), 0);
  $offset_last = variable_get('media_mediamosa_sync_offset_' . MediaMosaConnectorMediaWrapper::build_connection_id(), 0);

  mediamosa_ck::watchdog('_media_mediamosa_sync started, highest change/modified time @time_last.', array('@time_last' => $time_last ? format_date($time_last) : t('never')), WATCHDOG_DEBUG);

  // These two introduced just for statistic purposes.
  $assets_updated = 0;
  $assets_created = 0;

  // Really low what we are going to do here. 1st sync will take time. Lets not
  // choke cron.
  $limit = max(min(variable_get('media_mediamosa_cron_sync_throttle_cron_limit', 100), 199), 10);

  // Get the assets.
  $assets = MediaMosaConnectorMediaWrapper::request_current_assets_get($time_last, $offset_last, $limit + 1);
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
        _media_mediamosa_metadata_create_media($asset);
        $assets_created++;
      }
      else {
        // Found, update it.
        _media_mediamosa_metadata_update_media($asset, $media_id);
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
      $time_last = _media_mediamosa_date2unix($videotimestampmodified);
      variable_set('media_mediamosa_sync_time_' . MediaMosaConnectorMediaWrapper::build_connection_id(), $time_last);
      variable_set('media_mediamosa_sync_offset_' . MediaMosaConnectorMediaWrapper::build_connection_id(), $offset + 1);
    }
    else {
      // Next time we'll take everything that is new or changed.
      $time_last = _media_mediamosa_date2unix($videotimestampmodified) + 1;
      variable_set('media_mediamosa_sync_time_' . MediaMosaConnectorMediaWrapper::build_connection_id(), $time_last);
      variable_set('media_mediamosa_sync_offset_' . MediaMosaConnectorMediaWrapper::build_connection_id(), 0);
    }
  }

  mediamosa_ck::watchdog(
    'MediaMosa->Drupal sync run finished, There where @new_assets new assets of total @total_assets assets. Highest change/modified set to @time_last.',
    array(
      '@new_assets' => $assets_created,
      '@total_assets' => $assets_created + $assets_updated,
      '@time_last' => format_date($time_last),
    ),
    WATCHDOG_INFO
  );

  // Useful when we run this via GUI.
  return array(
    'processed_total' => $assets_created + $assets_updated,
    'assets_created' => $assets_created,
    'assets_updated' => $assets_updated,
  );
}

/**
 * Syncs a set of assets with our database.
 */
function _media_mediamosa_sync_mediamosa_to_drupal_by_range($offset, $limit) {
  $assets_created = 0;
  $assets_updated = 0;

  // Get the assets.
  $assets = MediaMosaConnectorMediaWrapper::request_current_assets_get(0, $offset, $limit);
  foreach ($assets as $asset) {
    $asset_id = $asset['asset_id'];
    $media_id = _media_mediamosa_get_media_id($asset_id);

    // Not found in our database? Then create it.
    if (!$media_id) {
      _media_mediamosa_metadata_create_media($asset);
      $assets_created++;
    }
    else {
      // Found, update it.
      _media_mediamosa_metadata_update_media($asset, $media_id);
      $assets_updated++;
    }
  }

  // Return next offset.
  return array('offset' => $offset + $limit, 'processed_total' => count($assets), 'assets_created' => $assets_created, 'assets_updated' => $assets_updated);
}

/**
 * Update Drupal media object by given asset info.
 *
 * @param array() $asset
 * @param int $media_id
 */
function _media_mediamosa_metadata_update_media($asset, $media_id) {
  // Processing the Dublin core metadata fields.
  $metadata = array_merge($asset['dublin_core'], $asset['qualified_dublin_core']);

  // To distinct data in Drupal and in Mediamosa we use drupal_ prefix for the Drupal side variables.

  $drupal_media = media_load($media_id);
  $drupal_uri = MediaMosaStreamWrapper::mediamosa_parse_url($drupal_media->uri);
  if (empty($drupal_uri)) {
    return;
  }
  $drupal_mediafile = $drupal_uri['mediafile_id'];

  $changes_counter = 0;

  foreach ($metadata as $key => $value) {
    $field_name = FIELD_PREFIX . $key;
    $drupal_ref = &$drupal_media->$field_name;

    $drupal_value = empty($drupal_ref) ? '' : $drupal_ref[LANGUAGE_NONE]['0']['value'];

    if ($drupal_value != $value) {
      $drupal_ref[LANGUAGE_NONE]['0']['value'] = is_array($value) ? $value[0] : $value;
      $changes_counter++;
    }
  }

  // We have done with DC/QDC fields. Now lets take care about mediafiles itself and mediafiles tech info.

  $mediafiles = MediaMosaConnectorMediaWrapper::request_get_asset_mediafiles($asset['asset_id']);

  if (empty($mediafiles)) {
    mediamosa_ck::watchdog('No mediafiles at all while looking for @mediafile, asset @asset_id.',
               array('@mediafile' => $drupal_mediafile, '@asset_id' => $asset['asset_id']), WATCHDOG_ERROR);
    return FALSE;
  }

  $mediafile = FALSE;
  foreach ($mediafiles as $item) {
    if ($item['mediafile_id'] == $drupal_mediafile) {
      $mediafile = $item;
      // We found current mediafile. Hope 99.99.... % cases will fall into this category.
      break;
    }
  }

  if (empty($mediafile)) {
    // We didn't find our file, lets use other file from this asset.
    // @todo: changing uri is not saved to DB by media_save(); we need direct write if we need this functionality.
    $mediafile = $mediafiles[0];
    mediamosa_ck::watchdog('Mediafile @mediafile disappeared, will use another one from this asset @asset_id. Media id is @media_id.',
             array('@mediafile' => $drupal_mediafile, '@asset_id' => $asset['asset_id'], '@media_id' => $media_id), WATCHDOG_NOTICE);
    $drupal_media->uri = 'mediamosa://asset/' . $asset['asset_id'] . '/' . $mediafile['mediafile_id'] . '/' . $mediafile['filename'];
    $changes_counter++;
  }

  $tech_metadata = $mediafile['metadata'];

  // The mediafile_id and is_original_file are special cases.
  // Mostly parameters can be taken directly from 'metadata' subarray, keys are the same.
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
  $size = $tech_metadata['width'] .'px * '. $tech_metadata['height'] .'px';
  $field_name = FIELD_PREFIX . 'size';
  if (empty($drupal_media->{$field_name}) || $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] != $size) {
    $drupal_media->{$field_name}[LANGUAGE_NONE]['0']['value'] = $size;
    $changes_counter++;
  }

  // And here goes transparent stuff.
  $meta_keys = array( 'file_duration', 'filesize', 'mime_type', 'container_type', 'fps', 'video_codec', );

  foreach ($meta_keys as $key) {
    $field_name = FIELD_PREFIX . $key;
    if (empty($drupal_media->{$field_name}) || $drupal_media->{$field_name}[LANGUAGE_NONE][0]['value'] != $tech_metadata[$key]) {
      $drupal_media->{$field_name}[LANGUAGE_NONE][0]['value'] = $tech_metadata[$key];
      $changes_counter++;
    }
  }

  if ($changes_counter) {
    try {
      media_save($drupal_media);
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog('Error on update media for media @media_id, asset @asset_id: @error.',
              array('@media_id' => $media_id, '@asset_id' => $asset['asset_id'], '@error' => $e->getMessage()), WATCHDOG_ERROR);
      return FALSE;
    }
    mediamosa_ck::watchdog('Updated @counter properties for media object @media_id while syncronizing with Mediamosa.',
               array('@counter' => $changes_counter, '@media_id' => $media_id), WATCHDOG_DEBUG);
  }
}

/**
 * Create Drupal media object in accordance with given asset info.
 *
 * @param array() $asset
 */
function _media_mediamosa_metadata_create_media($asset) {
  try {
    $mediafiles = MediaMosaConnectorMediaWrapper::request_get_asset_mediafiles($asset['asset_id']);

    $mediafile = FALSE;
    foreach ($mediafiles as $mediafile) {
      if ($mediafile['is_original_file'] == 'TRUE') {
        break;
      }
    }

    if ($mediafile && $mediafile['is_original_file'] == 'TRUE') {
      // We need to build the scheme uri. (mediamosa://asset/[id]/[id]/[filename]).
      $uri = strtr('mediamosa://asset/@asset_id/@mediafile_id/@filename', array('@asset_id' => $mediafile['asset_id'], '@mediafile_id' => $mediafile['mediafile_id'], '@filename' => $mediafile['filename']));

      // Save it using uri.
      $file = file_uri_to_object($uri);
      file_save($file);
    }
  }
  catch (Exception $e) {
    mediamosa_ck::watchdog('_media_mediamosa_metadata_create_media: cannot create file for URI @uri: @error.', array('@uri' => $uri, '@error' => $e->getMessage()), WATCHDOG_ERROR);
    return FALSE;
  }

  mediamosa_ck::watchdog('Created media object @media_id to reflect URI @uri.', array('@uri' => $uri, '@media_id' => $file->fid), WATCHDOG_DEBUG);

  // Call update right now to populate Dublin Core and technical metadata.
  _media_mediamosa_metadata_update_media($asset, $file->fid);

  return $file->fid;
}
