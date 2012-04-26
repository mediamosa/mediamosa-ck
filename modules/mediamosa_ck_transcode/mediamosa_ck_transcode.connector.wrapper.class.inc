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
   * Retrieving profile names.
   *
   * @deprecated: uses deprecated function response2array().
   *
   * @return profile names
   */
  static public function get_profile_names() {

    try {
      // Do request.
      $response = mediamosa_ck::request_get_fatal('transcode/profile');
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to update metadata; @message.', array('@message' => $e->getMessage()));
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
   * @param $asset_id
   *   ID of asset.
   * @param $owner_id
   *   If you already know an owner please supply; this will save from extra
   *   request.
   *
   * @return
   *   Array with jobs.
   */
  static public function get_asset_actual_jobs($asset_id, $owner_id = FALSE) {

    $uri = strtr('asset/@asset_id/joblist', array('@asset_id' => $asset_id));

    if (!$owner_id) {
      $owner_id = self::get_asset_owner($asset_id);
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
      mediamosa_ck::watchdog_error('Unable to get job list; @message.', array('@message' => $e->getMessage()));
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
  static public function delete_job($job_id, $owner_id) {

    // REST uri.
    $uri = strtr('job/@job_id/delete', array('@job_id' => $job_id));

    try {
      $result = mediamosa_ck::request_post_fatal($uri, array('user_id' => $owner_id));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete job; @message.', array('@message' => $e->getMessage()));
      return FALSE;
    }

    return $result->code == 200;

  }
}
