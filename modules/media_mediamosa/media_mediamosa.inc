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
 * All non-hooks are put in this file.
 */

/**
 * Job.
 */
define('MEDIA_MEDIAMOSA_JOB_TYPE', 'job_type');
define('MEDIA_MEDIAMOSA_JOB_TYPE_TRANSCODE', 'TRANSCODE');
define('MEDIA_MEDIAMOSA_JOB_TYPE_STILL', 'STILL');

define('MEDIA_MEDIAMOSA_JOB_STATUS', 'status');
define('MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED', 'FINISHED');
define('MEDIA_MEDIAMOSA_JOB_STATUS_FAILED', 'FAILED');
define('MEDIA_MEDIAMOSA_JOB_STATUS_WAITING', 'WAITING');

/**
 * Form callback for adding media via an upload form.
 */
function _media_mediamosa_upload_form($form, &$form_state, $types = NULL) {

  // Try to create upload ticket.
  $result = MediaMosaCkConnectorWrapper::request_create_upload_ticket();
  if (!$result) {
    $form['upload'] = array(
      '#markup' => t('Unable to upload to MediaMosa; @reason', array('@reason' => mediamosa_ck::get_connector_last_error_text())),
    );

    return $form;
  }

  // Get upload URL.
  $upload_url = $result['action'];

/*
  drupal 6;
  <input type="file" name="file"  class="form-file required" id="edit-file" size="60" />

  drupal 7
  <input type="file" id="edit-file" name="files[file]" size="60" class="form-file" />
*/

  // Because we override.
  $form_state['has_file_element'] = TRUE;

  // FIXME: Hack to make it work with single array file upload to mediamosa.
  // To make it work with MediaMosa single array upload result.
  $form['file'] = array(
    '#markup' => '<input type="file" id="edit-file" name="file" size="60" class="form-file" />',
    '#title' => t('Upload a new file to MediaMosa'),
  );

  // Redirect value, we need to redirect back after upload so we can add it to
  // Drupal.
  $form['redirect_uri'] = array(
    '#type' => 'hidden',
    '#value' => url('media_mediamosa/mediafile/uploaded/' . rawurlencode($result['asset_id']) . '/' . rawurlencode($result['mediafile_id']), array('absolute' => TRUE)),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['#action'] = $upload_url;

  // Merge with other options in for upload (turned off for now).
  $form = array_merge(module_invoke_all('media_mediamosa_upload_collect'), $form);

  return $form;
}

/**
 * Access for media.
 */
function _media_mediamosa_media_permission($fid, $permission) {
  // File type check.
  if (!_media_mediamosa_file_type_check($fid)) {
    return FALSE;
  }

  return (empty($permission) || !user_access($permission)) ? FALSE : TRUE;
}

/**
 * Provide asset ID to media ID mapping.
 * Media ID is the same as file ID in the file_managed table.
 *
 * @return array with asset_id as key and media_id as value, both are strings.
 * @see _media_mediamosa_get_media_id().
 */
function _media_mediamosa_asset_to_media_mapping() {

  // Setup static.
  $assetids_to_fids = &drupal_static(__FUNCTION__, array());

  if (!isset($assetids_to_fids)) {
    // Do a query.
    $query = db_select('file_managed', 'fm');
    $query->fields('fm', array('fid', 'uri'));
    $query->condition('uri', 'mediamosa://%', 'LIKE');
    $query->orderBy('fid', 'ASC');
    $result = $query->execute()->fetchAll();

    $assetids_to_fids = array();
    foreach ($result as $media) {
      $uri_parts = MediaMosaStreamWrapper::mediamosa_parse_url($media->uri);
      $assetids_to_fids[$uri_parts['asset_id']] = $media->fid;
    }
  }

  return $assetids_to_fids;
}

/**
 * Figure out how many assets are in file_managed.
 */
function _media_mediamosa_assets_in_file_managed() {
  $query = db_select('file_managed', 'fm');
  $query->condition('uri', 'mediamosa://%', 'LIKE');
  return $query->countQuery()->execute()->fetchField();
}

/**
 * Figure out how many assets are in MediaMosa.
 */
function _media_mediamosa_assets_in_mediamosa() {
  return MediaMosaConnectorMediaWrapper::request_get_number_of_assets();
}

/**
 * File type check.
 *
 * @param int $fid
 *   File id.
 */
function _media_mediamosa_file_type_check($fid) {
  if (!is_numeric($fid)) {
    return FALSE;
  }

  $media = media_load($fid);
  if (!$media || !isset($media->uri)) {
    return FALSE;
  }


  $uri = explode('://', $media->uri);

  return $uri[0] == 'mediamosa';
}

/**
 * Return media ID for a given asset ID.
 *
 * @return media ID AKA managed file ID, or FALSE if not found.
 * @see _media_mediamosa_asset_to_media_mapping().
 */
function _media_mediamosa_get_media_id($asset_id) {

  // Some cache.
  $assetids_to_fids = &drupal_static(__FUNCTION__, array());

  if (isset($assetids_to_fids[$asset_id])) {
    return $assetids_to_fids[$asset_id];
  }

  $query = db_select('file_managed', 'fm');
  $query->fields('fm', array('fid'));
  $query->condition('uri', 'mediamosa://asset/' . db_like($asset_id) .  '/%', 'LIKE');
  $query->orderBy('fid', 'ASC');
  $assetids_to_fids[$asset_id] = $query->execute()->fetchField();

  // When to many, remove the 1st in the array.
  // Need to be careful with memory.
  if (count($assetids_to_fids) > 10000) {
    array_shift($assetids_to_fids);
  }

  return $assetids_to_fids[$asset_id];
}

/**
 * Get running jobs.
 * And create a form item for that information.
 *
 * @param &$form
 * @param $mediafile_id
 * @param $job_type
 */
function _media_mediamosa_get_running_jobs(&$form, $mediafile_id, $job_type) {
  $jobs = MediaMosaCkConnectorWrapper::request_job_get($mediafile_id, mediamosa_ck::setting_user_id());
  $jobs_count = 0;
  if ($jobs->items->item) {
    foreach ($jobs->items->item as $job) {
      if ((string) $job->{MEDIA_MEDIAMOSA_JOB_STATUS} != MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED && (string) $job->{MEDIA_MEDIAMOSA_JOB_STATUS} != MEDIA_MEDIAMOSA_JOB_STATUS_FAILED && (string) $job->{MEDIA_MEDIAMOSA_JOB_TYPE} == $job_type) {
        $jobs_count++;
      }
    }
  }
  if ($jobs_count) {
    $form['running_jobs'] = array(
      '#type' => 'item',
      '#title' => t('Running @job_type jobs', array('@job_type' => drupal_strtolower($job_type),)),
      '#markup' => format_plural($jobs_count, 'There is @count @job_type job in progress.', 'There are @count @job_type jobs in progress.', array('@job_type' => drupal_strtolower($job_type))),
    );
  }
}

/**
 * Return the time in RFC 3339 format.
 * Eg. 2003-12-13T18:30:02Z
 * @link http://www.ietf.org/rfc/rfc3339.txt RFC 3339
 *
 * @param $time
 *   The time. If not given, the application uses the current time.
 *
 * @return date
 */
function _media_mediamosa_time_to_rfc_3339($time = NULL) {
  return $time ? gmdate('Y-m-d\TH:i:s\Z', $time) : gmdate('Y-m-d\TH:i:s\Z');
}

/**
 * Delete the transcoded mediafile of an asset.
 *
 * @param string $asset_id.
 *
 * @return array $mediafile_ids
 */
function _media_mediamosa_delete_mediafile_transcoded($asset_id) {
  $mediafile_ids = array();

  $asset = MediaMosaCkConnectorWrapper::request_get_asset($asset_id);

  if (isset($asset->items->item->mediafiles->mediafile) && $asset->items->item->mediafiles->mediafile) {
    foreach ($asset->items->item->mediafiles->mediafile as $mediafile) {
      if ((string) $mediafile->is_original_file != 'TRUE') {
        MediaMosaCkConnectorWrapper::mediafile_delete((string) $mediafile->mediafile_id, (string) $mediafile->owner_id);
      }
    }
  }

  return $mediafile_ids;
}

// Convert any date into unix timestamp.
function _media_mediamosa_date2unix($any_date) {
  // Parse the date, date_parse is new in 5.2.
  $date_parsed = date_parse($any_date);

  // Convert to integer
  return gmmktime($date_parsed['hour'], $date_parsed['minute'], $date_parsed['second'], $date_parsed['month'], $date_parsed['day'], $date_parsed['year']);
}
