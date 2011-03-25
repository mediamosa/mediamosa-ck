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
 *  Our global class for settings and static functions.
 */
class mediamosa_ck extends mediamosa_sdk {

  /**
   * Create or return the current connector.
   *
   * @param boolean $reset_session
   *  TRUE / FALSE to reset session.
   */
  static public function get_connector($reset_session = FALSE) {
    $mediamosa_connector = &drupal_static(__CLASS__ . '::' . __FUNCTION__, FALSE);

    // Create when not found.
    if (empty($mediamosa_connector)) {
      $mediamosa_connector = new MediaMosaCkConnectorWrapper();
    }

    // Reset session when needed.
    if ($reset_session) {
      $mediamosa_connector->reset_session();
    }

    return $mediamosa_connector;
  }

  /**
   * Wrapper for quicker logging.
   */
  static public function watchdog($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL, $type = 'MediaMosa Construction Kit') {
    $type = empty($type) ? 'MediaMosa Construction Kit' : 'MediaMosa Construction Kit - ' . $type;
    watchdog(substr($type, 0, 64), $message, $variables, $severity, $link);
  }

  /**
   * Wrapper for quick debug messages.
   */
  static public function debug($file, $line, $message, array $variables = array(), $severity = WATCHDOG_DEBUG, $link = NULL, $type = 'MediaMosa Construction Kit - Debug') {
    $variables = array('@file' => $file, '@line' => $line, '@message' => t($message, $variables));
    $message = '@message (In file @file on line @line).';
    drupal_set_message(t($message, $variables));
    watchdog(substr($type, 0, 64), $message, $variables, $severity, $link);
  }

  /**
   * Return the last error of the connector.
   */
  static public function get_connector_last_error() {
    $mediamosa_connector = self::get_connector();

    return $mediamosa_connector->get_last_error();
  }

  /**
   * Return the last error of the connector.
   */
  static public function get_connector_last_error_text() {
    $mediamosa_connector = self::get_connector();

    return $mediamosa_connector->get_last_error_text();
  }

  /**
   * Get the default user / owner for assets and mediafiles.
   */
  static public function setting_user_id() {
    global $user;

    return $user->uid;
  }

  /*
   * *
   * Do request to MediaMosa.
   *
   * @param string $uri
   *  The REST uri
   *
   * @param array $options
   *  Options for REST call.
   *
   * @param boolean $check
   *  Do result check.
   *
   * @return string
   */
  static public function request($uri, array $options = array(), $check = FALSE) {
    $result = FALSE;

    // Get the connection class object.
    $mediamosa_connector = self::get_connector();

    // Do the request.
    if ($mediamosa_connector) {
      $result = $mediamosa_connector->request($uri, $options);

      if ($check) {
        mediamosa_connector::check_result($result, FALSE, '%message');
      }
    }

    return $result;
  }

  /**
   * Do GET request to MediaMosa.
   *
   * @param string $uri
   *  The REST uri
   *
   * @param array $options
   *  Options for REST call.
   *
   * @param boolean $check
   *  Do result check.
   *
   * @return string
   */
  static public function request_get($uri, array $options = array(), $check = FALSE) {
    $options['method'] = 'GET';
    return self::request($uri, $options, $check);
  }

  /**
   * Do GET request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param string $uri
   *  The REST uri
   *
   * @param array $options
   *  Options for REST call.
   *
   * @param boolean $check
   *  Do result check.
   *
   * @return string
   */
  static public function request_get_fatal($uri, array $options = array(), $check = FALSE) {
    $options['method'] = 'GET';
    $options['fatal'] = TRUE;
    return self::request($uri, $options, $check);
  }

  /**
   * Do POST request to MediaMosa.
   *
   * @param string $uri
   *  The REST uri
   *
   * @param array $options
   *  Options for REST call.
   *
   * @param boolean $check
   *  Do result check.
   *
   * @return string
   */
  static public function request_post($uri, array $options = array(), $check = FALSE) {
    $options['method'] = 'POST';
    return self::request($uri, $options, $check);
  }

  /**
   * Do POST request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param string $uri
   *  The REST uri
   *
   * @param array $options
   *  Options for REST call.
   *
   * @param boolean $check
   *  Do result check.
   *
   * @return string
   */
  static public function request_post_fatal($uri, array $options = array(), $check = FALSE) {
    $options['method'] = 'POST';
    $options['fatal'] = TRUE;
    return self::request($uri, $options, $check);
  }

  /**
   * Return the setting for mediafile downloadable.
   */
  static public function setting_mediafile_downloadable() {
    return variable_get('mediamosa_ck_mediafile_downloadable', TRUE);
  }

  /**
   * Return the setting for mediafile creation of still after upload.
   */
  static public function setting_mediafile_upload_create_still() {
    return variable_get('mediamosa_ck_mediafile_upload_create_still', TRUE);
  }

  /**
   * Return the setting for mediafile tag on original.
   */
  static public function setting_mediafile_tag() {
    return variable_get('mediamosa_ck_mediafile_tag', '');
  }
}
