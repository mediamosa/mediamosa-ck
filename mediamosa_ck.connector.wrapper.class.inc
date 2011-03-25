<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
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
 * Our extended class on top of mediamosa_connector.
 */
class MediaMosaCkConnectorWrapper extends mediamosa_connector {

  /**
   * Check the connector settings.
   */
  static public function check_connection() {
    try {
      $result = mediamosa_ck::request_get_fatal('version');
    }
    catch (Exception $e) {
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Build the connection ID using the connector.
   */
  static public function build_connection_id() {
    $connector = mediamosa_ck::get_connector();

    return $connector->get_connection_id();
  }

  /**
   * Converts boolean to string ('TRUE' / 'FALSE').
   *
   * @param array $in
   *  Array with values to convert. May have other than specified in params.
   *
   * @param array $params
   *  The params to convert.
   */
  static public function bool2string(array $in, array $params = array()) {
    foreach ($params as $param) {
      assert(isset($in[$param]));
      $in[$param] = empty($in[$param]) ? 'FALSE' : 'TRUE';
    }

    return $in;
  }

  /**
   * Create a upload ticket and return the upload POST URL.
   */
  static public function request_create_upload_ticket(array $options = array()) {
    try {
      // Get the user_id for the owner.
      $user_id = mediamosa_ck::setting_user_id();

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('asset/create', array('user_id' => $user_id));
      $data_create_mediafile['asset_id'] = (string) $response->xml->items->item->asset_id;

      // Downloadable setting.
      $data_create_mediafile['is_downloadable'] = mediamosa_ck::setting_mediafile_downloadable() ? 'TRUE' : 'FALSE';

      // Create still after upload?
      $data_create_mediafile['create_still'] = mediamosa_ck::setting_mediafile_upload_create_still() ? 'TRUE' : 'FALSE';

      // Get optional tag.
      $tag = mediamosa_ck::setting_mediafile_tag();
      if (!empty($tag)) {
        $data_create_mediafile['tag'] = $tag;
      }

      // Create the mediafile
      $response = mediamosa_ck::request_post_fatal('mediafile/create', array('data' => $data_create_mediafile, 'user_id' => $user_id));
      $mediafile_id = (string) $response->xml->items->item->mediafile_id;

      // Create the upload ticket.
      $response = mediamosa_ck::request_post_fatal('mediafile/'. $mediafile_id . '/uploadticket/create', array('user_id' => $user_id));

      // Get the Action url from the result.
      $action = (string) $response->xml->items->item->action;

      return array(
        'asset_id' => $data_create_mediafile['asset_id'],
        'mediafile_id' => $mediafile_id,
        'action' => $action,
      );
    }
    catch (Exception $e) {
      return FALSE;
    }
  }

  /**
   * Get the mediafile.
   *
   * @param string $mediafile_id
   *  The ID of the mediafile to get.
   * @param array $options
   *  - show_stills (Default TRUE).
   *    TRUE / FALSE to include stills.
   */
  static public function request_get_mediafile($mediafile_id, array $options = array(), $reset = FALSE) {

    // Save results/
    $mediafiles = &drupal_static(__CLASS__ . '::' . __FUNCTION__, array());
    if ($reset) {
      $mediafiles = array();
    }
    elseif (empty($mediafiles[$mediafile_id])) {
      $options += array(
        'show_stills' => TRUE,
      );

      // Convert bool to strings.
      $options = self::bool2string($options, array('show_stills'));

      // Create the asset.
      $mediafiles[$mediafile_id] = mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id), $options)->xml;
    }

    return $mediafiles[$mediafile_id];
  }

  /**
   * Get the asset.
   *
   * @param string $asset_id
   *  ID of the asset to get.
   * @param $options
   *  - show_stills (Default TRUE).
   *    TRUE / FALSE to include stills.
   */
  static public function request_get_asset($asset_id, array $options = array()) {
    $options += array(
      'show_stills' => TRUE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('show_stills'));

    return mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id), array('data' => $options))->xml;
  }

  /**
   * Get the owner of the asset.
   *
   * @param string $asset_id
   *  The asset to get.
   * @param array $options
   *  Reserved.
   */
  static public function request_get_asset_owner($asset_id, array $options = array()) {
    $xml = self::request_get_asset($asset_id);
    return empty($xml->items->item->owner_id) ? '' : (string) $xml->items->item->owner_id;
  }

  /**
   * Create a play link.
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $user_id
   * @param array $options
   */
  static public function request_get_play_link($asset_id, $mediafile_id = '', $user_id = '', array $options = array()) {

    $play_links = &drupal_static(__CLASS__ . '::' . __FUNCTION__, array());

    try {
      // If now user_id is present, then we need the owner from the asset_id. We
      // don't fix this by using the is_app_admin, because it will then allow to
      // play unapropriate videos.
      if (empty($user_id)) {
        $user_id = self::request_get_asset_owner($asset_id);
      }

      $options['user_id'] = $user_id;
      $options['mediafile_id'] = $mediafile_id;

      if (!empty($play_links[$asset_id][$mediafile_id])) {
        return $play_links[$asset_id][$mediafile_id];
      }

      $xml = mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id) . '/play', array('data' => $options))->xml;

      $play_links[$asset_id][$mediafile_id] = empty($xml->items->item->output) ? FALSE : (string) $xml->items->item->output;
      return $play_links[$asset_id][$mediafile_id];
    }
    catch (Exception $e) {
      return FALSE;
    }
  }

  /**
   * Get job for mediafile.
   *
   * @param $mediafile_id
   *
   * @return $jobs
   */
  static public function request_job_get($mediafile_id, $user_id = NULL) {
    if (!$user_id) {
      $mediafile = MediaMosaCkConnectorWrapper::request_get_mediafile($mediafile_id);
      $user_id = (string) $mediafile->items->item->owner_id;
    }

    $data = array(
      'user_id' => $user_id,
    );

    return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id) . '/joblist', array('data' => $data))->xml;
  }

  /**
   * Asset delete.
   *
   * @param $asset_id
   * @param $user_id
   */
  static public function asset_delete($asset_id, $user_id) {
    // Check the parameters.
    if (empty($asset_id) || empty($user_id)) {
      return FALSE;
    }

    $data = array(
      'user_id' => $user_id,
      'delete' => 'cascade',
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/delete', array('data' => $data))->xml;
    }
    catch (Exception $e) {
      drupal_set_message('Unable to delete asset, reason; ' . $e->getMessage(), 'error');
    }

    return FALSE;
  }

  /**
   * Mediafile delete.
   *
   * @param $mediafile_id
   * @param $user_id
   */
  static public function mediafile_delete($mediafile_id, $user_id) {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    $data = array(
      'user_id' => $user_id,
      'delete' => 'cascade',
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/delete', array('data' => $data))->xml;
    }
    catch (Exception $e) {
      drupal_set_message('Unable to delete mediafile, reason; ' . $e->getMessage(), 'error');
    }

    return FALSE;
  }
}
