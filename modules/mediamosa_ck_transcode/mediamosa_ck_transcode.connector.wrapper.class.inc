<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Our wrapper around the mediamosa_connector.
 */

class MediaMosaCkConnectorTranscodeWrapper extends MediaMosaCkConnectorWrapper {
  /**
   * Get transcode profiles.
   *
   * @return $profiles
   */
  static public function transcode_profiles_get() {
    $data = array();

    try {
      // Do request.
      return mediamosa_ck::request_get_fatal('transcode/profile', array('data' => $data));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to retrieve the transcode profiles, reason; ' . $e->getMessage(), 'error');
    }

    return FALSE;
  }

  /**
   * Create transcoded mediafile.
   *
   * @param string $mediafile_id
   * @param string $user_id
   * @param array $data
   */
  static public function transcode_create($mediafile_id, $user_id, $data = array()) {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    $data += array(
      'user_id' => $user_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/transcode', array('data' => $data));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to start a transcode, reason; ' . $e->getMessage(), 'error');
    }

    return FALSE;
  }

  /**
   * Retrieving profile names.
   *
   * @return profile names
   */
  static public function request_get_profile_names() {

    try {
      // Do request.
      $response = mediamosa_ck::request_get_fatal('transcode/profile');
    }
    catch (Exception $e) {
      drupal_set_message('Unable to update metadata; ' . $e->getMessage(), 'error');
      return FALSE;
    }

    // Convert to array.
    $xml = mediamosa_connector::response2array($response);

    // Return our items.
    return !empty($xml['items']['item']) ? $xml['items']['item'] : array();
  }

  /**
   * Retrieving all non-finished jobs.
   *
   * @param string $asset_id ID of asset.
   * @param string $owner_id if you already know an owner please supply; this will save from extra request.
   *
   * @return array with jobs.
   */
  static public function request_get_asset_actual_jobs($asset_id, $owner_id = FALSE) {

    $uri = strtr('asset/@asset_id/joblist', array('@asset_id' => $asset_id));

    if (!$owner_id) {
      $owner_id = self::request_get_asset_owner($asset_id);
    }

    $data = array(
      'cql' => 'NOT status == "^FINISHED^"',
      'user_id' => $owner_id,
    );

    try {
      // Do request.
      $response = mediamosa_ck::request_get_fatal($uri, array('data' => $data));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to get job list; ' . $e->getMessage(), 'error');
      return FALSE;
    }

    $xml = mediamosa_connector::response2array($response);

    $jobs = !empty($xml['items']['item']) ? $xml['items']['item'] : array();

    return $jobs;

  }

  /**
   * Delete a particular job.
   *
   * @param string $job_id
   * @param string $owner_id
   *
   * @return bool.
   */
  static public function request_post_job_delete($job_id, $owner_id) {

    // REST uri.
    $uri = strtr('job/@job_id/delete', array('@job_id' => $job_id));

    try {
      $result = mediamosa_ck::request_post_fatal($uri, array('user_id' => $owner_id));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to delete job; ' . $e->getMessage(), 'error');
      return FALSE;
    }

    return $result->code == 200;

  }

  /**
   * Delete mediafile.
   *
   * @param string $job_id
   * @param string $owner_id
   *
   * @return bool.
   */
  static public function request_post_mediafile_delete($mediafile_id, $owner_id) {

    // REST uri.
    $uri = strtr('mediafile/@mediafile_id/delete', array('@mediafile_id' => $mediafile_id));

    try {
      $result = mediamosa_ck::request_post_fatal($uri, array('user_id' => $owner_id));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to delete mediafile; ' . $e->getMessage(), 'error');
      return FALSE;
    }

    return $result->code == 200;
  }
}
