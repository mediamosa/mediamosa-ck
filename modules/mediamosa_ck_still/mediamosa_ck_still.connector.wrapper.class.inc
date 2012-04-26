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

class MediaMosaCkConnectorStillWrapper extends MediaMosaCkConnectorWrapper {

  /**
   * Create a still for specific mediafile. Code moved to
   * MediaMosaCkConnectorWrapper.
   *
   * @param $mediafile_id
   *   The mediafile ID.
   * @param $user_id
   *   The owner of the mediafile.
   * @param $data
   *   Array with all the still parameters.
   *
   * @return mediamosa_connector_response
   *   Returns FALSE when failure or mediamosa_connector_response.
   */
  static public function still_create($mediafile_id, $user_id, array $data = array()) {
    return parent::create_mediafile_still($mediafile_id, $user_id, $data);
  }

  /**
   * Set still default.
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $still_id
   * @param string $user_id
   * @param array $data
   */
  static public function still_default($asset_id, $mediafile_id, $still_id, $user_id, array $data = array()) {
    // Check the parameters.
    if (empty($asset_id) || empty($mediafile_id) || empty($still_id) || empty($user_id)) {
      return FALSE;
    }

    $data += array(
      'user_id' => $user_id,
      'mediafile_id' => $mediafile_id,
      'still_id' => $still_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/still/default', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to start a still process, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Delete still.
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $still_id
   * @param string $user_id
   * @param array $data
   */
  static public function still_delete($asset_id, $mediafile_id, $still_id, $user_id, $data = array()) {
    // Check the parameters.
    if (empty($asset_id) || empty($mediafile_id) || empty($still_id) || empty($user_id)) {
      return FALSE;
    }

    $data += array(
      'user_id' => $user_id,
      'mediafile_id' => $mediafile_id,
      'still_id' => $still_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/still/delete', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to start a still process, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }
}
