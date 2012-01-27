<?php
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
 *  Our global class for settings and static functions.
 */
class mediamosa_ck extends mediamosa_sdk {

  /**
   * Create or return the current connector.
   *
   * @param $reset_session
   *  TRUE / FALSE to reset session.
   *
   * @return MediaMosaCkConnectorWrapper
   *   Returns the connector object.
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
   * This wrapper is indended when in the future if we want to tell the user
   * that something went wrong and log the problem. We might want to make
   * options for the admin in which we log the error, but will not bore the user
   * with the error.
   */
  static public function watchdog_error($message, array $variables = array()) {
    // Log it first.
    self::watchdog($message, $variables, WATCHDOG_ERROR);

    // Tell user.
    drupal_set_message(t($message, $variables), 'error');
  }

  /**
   * Wrapper for quicker logging.
   */
  static public function watchdog($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL, $type = 'MediaMosa CK') {
    $type = empty($type) ? 'MediaMosa CK' : 'MediaMosa CK - ' . $type;
    watchdog(substr($type, 0, 64), $message, $variables, $severity, $link);
  }

  /**
   * Wrapper for quick debug messages.
   */
  static public function debug($file, $line, $message, array $variables = array(), $severity = WATCHDOG_DEBUG, $link = NULL, $type = 'MediaMosa CK - Debug') {
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
   * Get the current session user ID to use for ownership or calls where
   * user_id is required.
   */
  static public function session_user_id() {
    global $user;

    // We use mail address as owner.
    return isset($user->mail) ? $user->mail : '';
  }

  /**
   * Builds static unique ID for entries in static array. Use for storing REST
   * call output based on the (asset/mediafile) ID and its options used for the
   * REST call.
   */
  static public function build_static_id($rest_options) {
    // Should make it unique enough.
    return sha1(print_r($rest_options, TRUE));
  }

  /**
   * Do request to MediaMosa.
   *
   * @param string $uri
   *   The REST uri
   * @param $options
   *   Options for REST call.
   * @param $check
   *   Do result check, will display drupal message when needed.
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return string
   */
  static public function request($uri, array $options = array(), $check = FALSE, $cached = FALSE, $reset = FALSE) {
    // Get the connection class object.
    $mediamosa_connector = self::get_connector();

    // Do the request.
    if ($mediamosa_connector) {
      // Only GET will be supported for caching.
      if ($options['method'] != 'GET' && $cached) {
        assert(0); // Do not set for other than GET.
        $cached = FALSE;
      }

      if ($cached) {
        // Reset the cache first?
        if ($reset) {
          drupal_static_reset('mediamosa_ck_requests');
        }

        $rest_calls = &drupal_static('mediamosa_ck_requests', array());
        $static_id = self::build_static_id($options);

        // Stored?
        if (!isset($rest_calls[$uri][$static_id])) {
          // Store it.
          $rest_calls[$uri][$static_id] = $mediamosa_connector->request($uri, $options);
        }

        // Check the result, shows drupal message when needed.
        if ($check) {
          mediamosa_connector::check_result($rest_calls[$uri][$static_id], FALSE, '%message');
        }

        // Return the result.
        return $rest_calls[$uri][$static_id];
      }

      $result = $mediamosa_connector->request($uri, $options);
      if ($check) {
        mediamosa_connector::check_result($result, FALSE, '%message');
      }

      return $result;
    }
    elseif (!empty($options['fatal'])) {
      throw new Exception('Unable to load connector');
    }
  }

  /**
   * Do GET request to MediaMosa.
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check.
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return
   *   The response object.
   */
  static public function request_get($uri, array $options = array(), $check = FALSE, $cached = FALSE, $reset = FALSE) {
    $options['method'] = 'GET';
    return self::request($uri, $options, $check, $cached, $reset);
  }

  /**
   * Do cached GET request to MediaMosa.
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check.
   * @param $reset
   *   Reset the cache.
   *
   * @return
   *   The response object.
   */
  static public function request_get_cached($uri, array $options = array(), $check = FALSE, $reset = FALSE) {
    return self::request_get($uri, $options, $check, TRUE, $reset);
  }

  /**
   * Do GET request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param $uri
   *   The REST uri
   * @param $options
   *   Options for REST call.
   * @param $check
   *   Do result check.
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return
   *   The response object.
   */
  static public function request_get_fatal($uri, array $options = array(), $check = FALSE, $cached = FALSE, $reset = FALSE) {
    $options['method'] = 'GET';
    $options['fatal'] = TRUE;
    return self::request($uri, $options, $check, $cached, $reset);
  }

  /**
   * Do GET request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param $uri
   *   The REST uri
   * @param $options
   *   Options for REST call.
   * @param $check
   *   Do result check.
   * @param $reset
   *   Reset the cache.
   *
   * @return
   *   The response object.
   */
  static public function request_get_fatal_cached($uri, array $options = array(), $check = FALSE, $reset = FALSE) {
    return self::request_get_fatal($uri, $options, $check, TRUE, $reset);
  }

  /**
   * Do POST request to MediaMosa.
   *
   * @param $uri
   *   The REST uri
   * @param $options
   *   Options for REST call.
   * @param $check
   *   Do result check.
   *
   * @return
   *   The response object.
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
   * @param array $options
   *  Options for REST call.
   * @param boolean $check
   *  Do result check.
   *
   * @return
   *   The response object.
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

  /**
   * Opposite of parse_url.
   *
   * @param $urls
   * @param $return_parts
   *  Return the array with host and uri.
   */
  static public function build_url($urls, $return_parts = FALSE) {

    $result = array(
      'host' => array($urls['scheme'] . '://'),
      'uri' => array(),
    );

    // Setup default.
    $urls += array(
      'user' => '',
      'pass' => '',
      'port' => 0,
      'path' => '',
      'query' => '',
      'fragment' => '',
    );

    // Add user : pass
    if ($urls['user'] != '' || $urls['pass'] != '') {
      $result['host'][] = $urls['user'];
      $result['host'][] = ':';
      $result['host'][] = $urls['pass'];
      $result['host'][] = '@';
    }

    // Host.
    $result['host'][] = trim($urls['host'], '/');

    // Optional port
    if ($urls['port']) {
      $result['host'][] = ':' . $urls['port'];
    }

    // Path.
    if ($urls['path'] != '') {
      $result['uri'][] = '/' . trim($urls['path'], '/');
    }

    // Query.
    if ($urls['query'] != '') {
      $result['uri'][] = '?' . $urls['query'];
    }

    // Fragment.
    if ($urls['fragment'] != '') {
      $result['uri'][] = $urls['fragment'];
    }

    // Now rebuild url.
    return $return_parts ? array('host' => implode('', $result['host']), 'uri' => implode('', $result['uri'])) : (implode('', $result['host']) . implode('', $result['uri']));
  }

  /**
   * Wrapper around http_build_query().
   *
   * @param array $query
   */
  static public function http_build_query($query) {
    return strtr(http_build_query($query, '', '&'), array('%5B' => '[', '%5D' => ']', '%5b' => '[', '%5d' => ']'));
  }

  /**
   * Opposite of http_build_query()
   *
   * @param $query_str
   */
  static public function parse_query($query_str) {
    // Split the url.
    $query = array();

    // Split values.
    foreach (explode('&', $query_str) as $valuepair) {
      if (strpos($valuepair, '=')) {
        list($name, $value) = explode('=', $valuepair, 2);
        $query[urldecode($name)][] = urldecode($value);
      }
      else {
        $query[urldecode($valuepair)][] = '';
      }
    }

    // Now make single item queries with 0 keys no array.
    foreach ($query as $name => $value) {
      if (count($value) == 1 && key($value) == 0) {
        $query[$name] = reset($value);
      }
    }

    return $query;
  }

  /**
   * Convert any date into unix timestamp.
   *
   * @param $any_date
   *   The date to convert.
   *
   * @return
   *   The number of seconds since 1970 (UTC/GMT).
   */
  static public function date2unix($any_date) {
    // Parse the date, date_parse is new in 5.2.
    $date_parsed = date_parse($any_date);

    // Convert to integer
    return gmmktime($date_parsed['hour'], $date_parsed['minute'], $date_parsed['second'], $date_parsed['month'], $date_parsed['day'], $date_parsed['year']);
  }

  /**
   * Converts mediafile duration format into unix timestamp.
   *
   * @param $any_date
   *   The date to convert.
   *
   * @return
   *   The number of seconds since 1970 (UTC/GMT).
   */
  static public function duration2unix($mediafile_duration) {
    // Split into pieces.
    list($hour, $minute, $second, $milisecond) = preg_split("/[:.]/", $mediafile_duration, 4);

    // Convert to # of seconds.
    return ((int) $hour * 3600) + ((int) $minute * 60) + (int) $second; + round((int) $milisecond / 100);
  }

  /**
   * This function will return the active metadata fields on the active
   * MediaMosa connection. For now we return DC/QDC, but in future we need to
   * find out or query MediaMosa what metadata is allowed. MediaMosa does not
   * yet have a REST call that will return the metadata fields in MediaMosa.
   */
  static public function get_metadata_fields() {
    // The properties to create.
    static $properties = array(
      'dc' => array(
        'title' => 'Dublin Core',
        'xpath' => 'dublin_core',
        'fields' => array(
          'type' => 'CHAR',
          'format' => 'CHAR',
          'language' => 'CHAR',
          'title' => 'CHAR',
          'creator' => 'CHAR',
          'publisher' => 'CHAR',
          'subject' => 'CHAR',
          'description' => 'CHAR',
          'contributor' => 'CHAR',
          'date' => 'DATETIME',
          'identifier' => 'CHAR',
          'source' => 'CHAR',
          'relation' => 'CHAR',
          'coverage_temporal' => 'CHAR',
          'coverage_spatial' => 'CHAR',
          'rights' => 'CHAR',
        ),
      ),
      'qdc' => array(
        'title' => 'Qualified Dublin Core',
        'xpath' => 'qualified_dublin_core',
        'fields' => array(
          'title_alternative' => 'CHAR',
          'description_abstract' => 'CHAR',
          'created' => 'DATETIME',
          'issued' => 'DATETIME',
          'hasformat' => 'CHAR',
          'isformatof' => 'CHAR',
          'format_medium' => 'CHAR',
          'format_extent' => 'CHAR',
          'license' => 'CHAR',
          'rightsholder' => 'CHAR',
          'isreferencedby' => 'CHAR',
        ),
      ),
    );

    return $properties;
  }

  /**
   * Returns the metadata field that is used for title. Must be in format as
   * SET.NAME and must be metadata.
   *
   * @see get_asset_title()
   */
  static public function get_metadata_name_title() {
    return 'dc.title';
  }

  /**
   * Returns the metadata field that is used for title. Must be in format as
   * SET.NAME and must be metadata.
   *
   * @see get_asset_description()
   */
  static public function get_metadata_name_description() {
    return 'dc.description';
  }

  /**
   * Return the fields that are used for searching.
   */
  static public function get_metadata_search_fields() {
    return array('dc.title', 'dc.description');
  }

  /**
   * Returns the (trimmed) title of the asset using the metadata.
   *
   * @param object $asset
   *   The asset in simplexmlelement or other object.
   * @param $default
   *   Default value when empty (=== '').
   *
   * @return
   *   The title or ''.
   *
   * @see get_metadata_name_title()
   * @see get_metadata_name_description()
   */
  static public function get_asset_title($asset, $default = '<no title>') {
    return trim(self::get_asset_value($asset, self::get_metadata_name_title(), $default));
  }

  /**
   * Returns the description of the asset using the metadata.
   *
   * @param object $asset
   *   The asset in simplexmlelement or other object.
   * @param $default
   *   Default value when empty (=== '').
   *
   * @return
   *   The title or ''.
   *
   * @see get_metadata_name_title()
   * @see get_metadata_name_description()
   */
  static public function get_asset_description($asset, $default = '<no description>') {
    return self::get_asset_value($asset, self::get_metadata_name_description(), $default);
  }

  /**
   * Returns the a value of an asset using it scope name ([set].[name]). Used
   * now for retieving metadata values.
   *
   * @param object $asset
   *   The asset in simplexmlelement or other object.
   * @param $field
   *   The name of the field to retrieve.
   * @param $default
   *   The default value when value was not found or empty string (=== '').
   *
   * @return
   *   The value or default.
   *
   * @see get_metadata_name_title()
   */
  static public function get_asset_value($asset, $field, $default = '') {
    // Inside XML result?
    if (isset($asset->items)) {
      return self::get_asset_value($asset->items->item, $field, $default);
    }

    // Might be object that already has it set as [set].[name]
    if (!empty($asset->{$field})) {
      return $asset->{$field};
    }

    // See if field contains set.
    if (strpos($field, '.') === FALSE) {
      return $default;
    }

    // Need parts.
    list($set, $name) = explode('.', $field, 2);

     // Need metadata info.
    $metadata_fields = self::get_metadata_fields();

    // Field name is known metadata?
    if ($asset instanceof SimpleXMLElement && isset($metadata_fields[$set]) && !empty($metadata_fields[$set]['fields'][$name])) {
      $xpath = $metadata_fields[$set]['xpath'] . '/' . $name;
      if ($asset->xpath($xpath)) {
        return (string) reset($asset->xpath($xpath));
      }
    }

    // Not found. In future version we can extend it more 'normal' fields.

    return $default;
  }

  /**
   * Lookup the Drupal user using his name. Although I would like to use
   * user_load_by_name(), I better use something I can pre-cache when a lot of
   * users need to be looked up.
   *
   * @param $name
   *   The name of the Drupal user.
   *
   * @return
   *   The user array.
   */
  static public function lookup_user_with_name($name) {
    // Precache.
    self::precache_users_with_names(array($name));

    // Get the users.
    $users_names = &drupal_static('mediamosa_ck_cache_drupal_users_names', array());

    // Get the user from the cache.
    return isset($users_names[$name]) ? $users_names[$name] : NULL;
  }

  /**
   * Lookup the Drupal users using his name. Although I would like to use
   * user_load_by_name(), I better use something I can pre-cache when a lot of
   * users need to be looked up. Also we only need mail, uid and name from the
   * users table.
   *
   * @param $names
   *   The names of the Drupal users to precache.
   */
  static public function precache_users_with_names(array $names) {
    // Store static. Storing it twice does not cost lost more memory, as the
    // users are objects, and these are referenced.
    $users_names = &drupal_static('mediamosa_ck_cache_drupal_users_names', array());
    $users_mails = &drupal_static('mediamosa_ck_cache_drupal_users_mails', array());
    $query = NULL;

    // Now see who we miss.
    foreach ($names as $name) {
      if (!isset($users_names[$name])) {
        if (!isset($query)) {
          $query = db_select('users', 'u')->fields('u', array('mail', 'uid', 'name'));
        }

        $query->condition('name', $name);
      }
    }

    // Any new users not in cache?
    if (isset($query)) {
      $result = $query->execute();
      foreach ($result as $user) {
        $users_names[$user->name] = $user;
        $users_mails[$user->mail] = $user;
      }
    }
  }

  /**
   * Lookup the Drupal user using his mail address.
   *
   * @param $mail
   *   The mail address of the Drupal user.
   *
   * @return
   *   The user array.
   */
  static public function lookup_user_with_mail($mail) {
    // Precache.
    self::precache_users_with_mails(array($mail));

    // Get the users.
    $users_mails = &drupal_static('mediamosa_ck_cache_drupal_users_mails', array());

    // Get the user from the cache.
    return isset($users_mails[$mail]) ? $users_mails[$mail] : NULL;
  }

  /**
   * Lookup the Drupal users using his name. Although I would like to use
   * user_load_by_name(), I better use something I can pre-cache when a lot of
   * users need to be looked up. Also we only need mail, uid and name from the
   * users table.
   *
   * @param $mails
   *   The mail address of the Drupal users to precache.
   */
  static public function precache_users_with_mails(array $mails) {
    // Store static. Storing it twice does not cost lost more memory, as the
    // users are objects, and these are referenced.
    $users_names = &drupal_static('mediamosa_ck_cache_drupal_users_names', array());
    $users_mails = &drupal_static('mediamosa_ck_cache_drupal_users_mails', array());
    $query = NULL;

    // Now see who we miss.
    foreach ($mails as $mail) {
      if (!isset($users_mails[$mail])) {
        if (!isset($query)) {
          $query = db_select('users', 'u')->fields('u', array('mail', 'uid', 'name'));
        }

        $query->condition('mail', $mail);
      }
    }

    // Any new users not in cache?
    if (isset($query)) {
      $result = $query->execute();
      foreach ($result as $user) {
        $users_names[$user->name] = $user;
        $users_mails[$user->mail] = $user;
      }
    }
  }
}
