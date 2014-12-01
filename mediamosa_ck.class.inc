<?php
/**
 * @file
 * Global class for settings and static functions.
 */

class mediamosa_ck extends mediamosa_sdk {
  // ------------------------------------------------------------------- Consts.
  // Permissions.
  const PERMISSION_ADMIN_CONFIGURATION = 'mediamosa-ck admin configuration';
  const PERMISSION_ADD_MEDIA = 'add media to mediamosa';

  // ---------------------------------------------------------------- Functions.
  /**
   * Create or return the current connector.
   *
   * @param boolean $reset_session
   *   TRUE / FALSE to reset session.
   *
   * @return MediaMosaCkConnectorWrapper
   *   Returns the connector object.
   */
  public static function get_connector($reset_session = FALSE) {
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
   * Display an user friendly error.
   *
   * In case of responding to an exception, use set_message_exception() instead.
   *
   * This wrapper is indended when in the future if we want to tell the user
   * that something went wrong and log the problem. We might want to make
   * options for the admin in which we log the error, but will not bore the user
   * with the error.
   *
   * @see set_message_exception()
   */
  public static function watchdog_error($message, array $variables = array()) {
    // Log it first.
    self::watchdog($message, $variables, WATCHDOG_ERROR);

    // Tell user.
    drupal_set_message(t($message, $variables), 'error');
  }

  /**
   * Display an readable version of the exception for the user.
   *
   * Will also log the exception.
   *
   * @param Exception $exception
   *   The exception that is going to be logged.
   * @param string $message
   *   The message that explains when or where the exception occurred.
   * @param array $variables
   *   Array of variables to replace in the message.
   */
  public static function set_message_exception(Exception $e, $message, array $variables = array()) {
    // Log it first.
    self::watchdog_exception($e);
    $message = t($message, $variables);
    // Tell user.
    $variables = array(
      '!message' => $message,
      '!message_exception' => $e->getMessage(),
    );
    drupal_set_message(strtr('!message: !message_exception', $variables), 'error');
  }

  /**
   * Wrapper for quicker logging.
   */
  public static function watchdog($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL, $type = 'MediaMosa CK') {
    $type = empty($type) ? 'MediaMosa CK' : 'MediaMosa CK - ' . $type;
    watchdog(substr($type, 0, 64), $message, $variables, $severity, $link);
  }

  /**
   * Logs an exception.
   *
   * This is a wrapper function for watchdog() which automatically decodes an
   * exception.
   *
   * @param Exception $exception
   *   The exception that is going to be logged.
   * @param string $message
   *   The message to store in the log. If empty, a text that contains all useful
   *   information about the passed-in exception is used.
   * @param array $variables
   *   Array of variables to replace in the message on display. Defaults to the
   *   return value of drupal_decode_exception().
   * @param string $severity
   *   The severity of the message, as per RFC 3164.
   * @param string $link
   *   A link to associate with the message.
   * @param string $type
   *   The category to which this message belongs.
   */
  public static function watchdog_exception(Exception $exception, $message = NULL, array $variables = array(), $severity = WATCHDOG_ERROR, $link = NULL, $type = 'MediaMosa CK') {
    $type = empty($type) ? 'MediaMosa CK' : 'MediaMosa CK - ' . $type;
    watchdog_exception($type, $exception, $message, $variables, $severity, $link);
  }

  /**
   * Wrapper for quick debug messages.
   */
  public static function debug($file, $line, $message, array $variables = array(), $severity = WATCHDOG_DEBUG, $link = NULL, $type = 'MediaMosa CK - Debug') {
    $variables = array(
      '@file' => $file,
      '@line' => $line,
      '@message' => t($message, $variables),
    );
    $message = '@message (In file @file on line @line).';
    drupal_set_message(t($message, $variables));
    watchdog(substr($type, 0, 64), $message, $variables, $severity, $link);
  }

  /**
   * Return the last error of the connector.
   */
  public static function get_connector_last_error() {
    $mediamosa_connector = self::get_connector();

    return $mediamosa_connector->get_last_error();
  }

  /**
   * Return the last error of the connector.
   */
  public static function get_connector_last_error_text() {
    $mediamosa_connector = self::get_connector();

    return $mediamosa_connector->get_last_error_text();
  }

  /**
   * Get current session user ID.
   *
   * Get the current session user ID to use for ownership or calls where
   * user_id is required.
   */
  public static function session_user_id() {
    global $user;

    // We use mail address as owner.
    if (variable_get('mediamosa_ck_asset_owner', 'mail') == 'mail') {
      return isset($user->mail) ? $user->mail : '<anonymous>';
    }
    else {
      return isset($user->name) ? $user->name : '<anonymous>';
    }
  }

  /**
   * Builds static unique ID.
   *
   * Builds static unique ID for entries in static array. Use for storing REST
   * call output based on the (asset/mediafile) ID and its options used for the
   * REST call.
   */
  public static function build_static_id($rest_options) {
    // Should make it unique enough.
    return sha1(print_r($rest_options, TRUE));
  }

  /**
   * Do request to MediaMosa.
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check, will display drupal message when needed.
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return string
   *   return result.
   */
  public static function request($uri, array $options = array(), $check = FALSE, $cached = FALSE, $reset = FALSE) {
    // Get the connection class object.
    $mediamosa_connector = self::get_connector();

    // Do the request.
    if ($mediamosa_connector) {
      // Only GET will be supported for caching.
      if ($options['method'] != 'GET' && $cached) {
        // Do not set for other than GET.
        assert(0);
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
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return object
   *   The response object.
   */
  public static function request_get($uri, array $options = array(), $check = FALSE, $cached = FALSE, $reset = FALSE) {
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
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return mediamosa_connector_response
   *   The response object.
   */
  public static function request_get_cached($uri, array $options = array(), $check = FALSE, $reset = FALSE) {
    return self::request_get($uri, $options, $check, TRUE, $reset);
  }

  /**
   * Do GET request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check.
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return mediamosa_connector_response
   *   The response object.
   */
  public static function request_get_fatal($uri, array $options = array(), $check = FALSE, $cached = FALSE, $reset = FALSE) {
    $options['method'] = 'GET';
    $options['fatal'] = TRUE;
    return self::request($uri, $options, $check, $cached, $reset);
  }

  /**
   * Do GET request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return mediamosa_connector_response
   *   The response object.
   */
  public static function request_get_fatal_cached($uri, array $options = array(), $check = FALSE, $reset = FALSE) {
    return self::request_get_fatal($uri, $options, $check, TRUE, $reset);
  }

  /**
   * Do POST request to MediaMosa.
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check.
   *
   * @return mediamosa_connector_response
   *   The response object.
   */
  public static function request_post($uri, array $options = array(), $check = FALSE) {
    $options['method'] = 'POST';
    return self::request($uri, $options, $check);
  }

  /**
   * Do POST request to MediaMosa but any error is fatal (expecting 601 code).
   *
   * @param string $uri
   *   The REST uri
   * @param array $options
   *   Options for REST call.
   * @param boolean $check
   *   Do result check.
   *
   * @return object
   *   The response object.
   */
  public static function request_post_fatal($uri, array $options = array(), $check = FALSE) {
    $options['method'] = 'POST';
    $options['fatal'] = TRUE;
    return self::request($uri, $options, $check);
  }

  /**
   * Return the setting for mediafile downloadable.
   */
  public static function setting_mediafile_downloadable() {
    return variable_get('mediamosa_ck_mediafile_downloadable', TRUE);
  }

  /**
   * Return the setting for mediafile creation of still after upload.
   */
  public static function setting_mediafile_upload_create_still() {
    return variable_get('mediamosa_ck_mediafile_upload_create_still', TRUE);
  }

  /**
   * Return the setting for mediafile tag on original.
   */
  public static function setting_mediafile_tag() {
    return variable_get('mediamosa_ck_mediafile_tag', '');
  }

  /**
   * Return the setting for mediafile is private.
   */
  public static function setting_mediafile_isprivate() {
    return variable_get('mediamosa_ck_mediafile_isprivate', FALSE);
  }

  /**
   * Opposite of parse_url.
   *
   * @param array $urls
   *
   * @param boolean $return_parts
   *   Return the array with host and uri.
   */
  public static function build_url($urls, $return_parts = FALSE) {

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

    // Add user:pass.
    if ($urls['user'] != '' || $urls['pass'] != '') {
      $result['host'][] = $urls['user'];
      $result['host'][] = ':';
      $result['host'][] = $urls['pass'];
      $result['host'][] = '@';
    }

    // Host.
    $result['host'][] = trim($urls['host'], '/');

    // Optional port.
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
    return $return_parts ? array(
      'host' => implode('', $result['host']),
      'uri' => implode('', $result['uri'])) : (implode('', $result['host']) . implode('', $result['uri'])
    );
  }

  /**
   * Wrapper around http_build_query().
   *
   * @param array $query
   */
  public static function http_build_query($query) {
    return strtr(http_build_query($query, '', '&'), array(
        '%5B' => '[',
        '%5D' => ']',
        '%5b' => '[',
        '%5d' => ']',
      )
    );
  }

  /**
   * Opposite of http_build_query().
   *
   * @param string $query_str
   */
  public static function parse_query($query_str) {
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
   * @param date $any_date
   *   The date to convert.
   *
   * @return int
   *   The number of seconds since 1970 (UTC/GMT).
   */
  public static function date2unix($any_date) {
    // Parse the date, date_parse is new in 5.2.
    $date_parsed = date_parse($any_date);

    // Convert to integer.
    return gmmktime($date_parsed['hour'], $date_parsed['minute'], $date_parsed['second'], $date_parsed['month'], $date_parsed['day'], $date_parsed['year']);
  }

  /**
   * Converts mediafile duration format into unix timestamp.
   *
   * @param date $any_date
   *   The date to convert.
   *
   * @return int
   *   The number of seconds since 1970 (UTC/GMT).
   */
  public static function duration2unix($mediafile_duration) {

    // Split into pieces.
    if (preg_match('/(?P<hours>\d+):(?P<minutes>\d+):(?P<seconds>\d+).(?P<milliseconds>\d+)/', $mediafile_duration, $matches)) {
      $hours = isset($matches['hours']) ? $matches['hours'] : '';
      $minutes = isset($matches['minutes']) ? $matches['minutes'] : '';
      $seconds = isset($matches['seconds']) ? $matches['seconds'] : '';
      $milliseconds = isset($matches['milliseconds']) ? $matches['milliseconds'] : '';
      // Convert to # of seconds.
      return ((int) $hours * 3600) + ((int) $minutes * 60) + (int) $seconds + round((int) $milliseconds / 100);
    }
    return 0;
  }

  /**
   * Get active metadata fields.
   *
   * This function will return the active metadata fields on the active
   * MediaMosa connection. For now we return DC/QDC, but in future we need to
   * find out or query MediaMosa what metadata is allowed. MediaMosa does not
   * yet have a REST call that will return the metadata fields in MediaMosa.
   */
  public static function get_metadata_fields() {

    $metadata_fields = &drupal_static('mediamosa_ck_metadata_fields');
    if (!isset($metadata_fields)) {
      // The metadata fields.
      $metadata_fields = array(
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
        'asset' => array(
          'title' => 'Asset',
          'xpath' => 'asset',
          'fields' => array(
            'tag' => 'CHAR',
          ),
        ),
      );

      // Give chance for others to change/add the metadata fields.
      drupal_alter('mediamosa_ck_metadata_fields', $metadata_fields);
    }

    return $metadata_fields;
  }

  /**
   * Returns the metadata for display usage.
   *
   * @param object $asset
   *   The asset response object.
   */
  public static function get_metadata_asset($asset, array $options = array()) {

    $options += array(
      'datetime' => array(),
    );

    $options['datetime'] += array(
      'type' => 'medium',
      'format' => '',
      // Time is stored as GMT.
      'timezone' => 'GMT',
      'langcode' => NULL,
     );

    $metadata = array();
    foreach (static::get_metadata_fields() as $context => $set) {
      if (isset($asset->{$set['xpath']})) {
        $metadata[$context] = array(
          'title' => $set['title'],
          'data' => array(),
        );
        foreach ($set['fields'] as $field => $type) {
          $metadata[$context]['data'][$field] = array('label' => static::get_metadata_label($field));
          if (isset($asset->{$set['xpath']}->{$field})) {
            foreach ($asset->{$set['xpath']}->{$field} as $value) {
              switch ($type) {
                case 'CHAR':
                  $value = (string) $value;
                  if ($value !== '') {
                    $metadata[$context]['data'][$field]['label'] = static::get_metadata_label($field);
                    $metadata[$context]['data'][$field]['contents'][] = (string) $value;
                  }
                  break;

                case 'DATETIME':
                  $value = (string) $value;
                  if ($value !== '') {
                    $metadata[$context]['data'][$field]['label'] = static::get_metadata_label($field);
                    $metadata[$context]['data'][$field]['contents'][] = mediamosa_sdk::datestamp2date($value, $options['datetime']['type'], $options['datetime']['format'], $options['datetime']['timezone'], $options['datetime']['langcode']);
                  }
                  break;

                case 'INT':
                  $value = (string) $value;
                  if ($value !== '') {
                    $metadata[$context]['data'][$field]['label'] = static::get_metadata_label($field);
                    $metadata[$context]['data'][$field]['contents'][] = (int) $value;
                  }
                  break;
              }
            }
          }
        }
      }
    }

    return $metadata;
  }

  /**
   * Convert metadata name into usable label for UI.
   *
   * @param string $name
   *   The name of the metadata field. Can be in format [contextset].[name],
   *   for example; 'dc.title'.
   *
   * @return string
   *   The label to use.
   */
  public static function get_metadata_label($metadata_name) {
    // Contains context set?
    if (strpos($metadata_name, '.') !== FALSE) {
      list(, $label) = explode('.', $metadata_name, 2);
    }
    else {
      $label = $metadata_name;
    }

    $label_fixes = array(
      'hasformat' => 'has format',
      'isreferencedby' => 'is referenced by',
      'isformatof' => 'is format of',
      'videotimestamp' => 'video timestamp',
      'videotimestampmodified' => 'video modified',
      'app_id_search' => 'app id',
      'title_alternative' => 'alternative title',
    );

    // Fix possible values.
    $label = (isset($label_fixes[$label]) ? $label_fixes[$label] : $label);

    // Remove _ and -.
    $label = drupal_ucfirst(str_replace(array('_', '-'), ' ', $label));

    return t($label);
  }

  /**
   * Return the selection list for metadata.
   *
   * @return array
   *   An array that can be used directly as SELECT #options array.
   */
  public static function get_metadata_fields_options() {
    // Empty first.
    $options = &drupal_static('mediamosa_ck_metadata_options', array());

    if (empty($options)) {
      $metadata_definition = self::get_metadata_fields();

      // Build the select.
      foreach ($metadata_definition as $metadata_group => $metadata_set) {
        $group_title = $metadata_set['title'];
        foreach (array_keys($metadata_set['fields']) as $name) {
          $name = $metadata_group . '.' . $name;
          $options[$group_title][$name] = self::get_metadata_label($name);
        }
      }
    }

    return $options;
  }

  /**
   * Returns the metadata field that is used for title.
   *
   * Must be in format as SET.NAME and must be metadata.
   *
   * @see get_asset_title()
   */
  public static function get_metadata_name_title() {
    return 'dc.title';
  }

  /**
   * Returns the metadata field that is used for title.
   *
   * Must be in format as SET.NAME and must be metadata.
   *
   * @see get_asset_description()
   */
  public static function get_metadata_name_description() {
    return 'dc.description';
  }

  /**
   * Return the fields that are used for searching.
   */
  public static function get_metadata_search_fields() {
    return array('dc.title', 'dc.description');
  }

  /**
   * Returns the (trimmed) title of the asset using the metadata.
   *
   * @param object $asset
   *   The asset in simplexmlelement or other object.
   * @param string $default
   *   Default value when empty (=== '').
   *
   * @return string
   *   The title or ''.
   *
   * @see get_metadata_name_title()
   * @see get_metadata_name_description()
   */
  public static function get_asset_title($asset, $default = '') {
    // Inside XML result?
    if (isset($asset->items)) {
      return self::get_asset_title($asset->items->item, $default);
    }


    $default = trim($default) === '' ? t('@asset_id', array('@asset_id' => (string) $asset->asset_id)) : $default;
    $title = trim(self::get_asset_value($asset, self::get_metadata_name_title()));
    if ($title !== '') {
      return $title;
    }

    // Return filename if we have one.
    if (isset($asset->mediafile_filename) && !empty($asset->mediafile_filename)) {
      return (string) $asset->mediafile_filename;
    }

    // See if asset is synced to filemanaged.
    $file = mediamosa_ck::get_filemanaged_from_assets(array((string) $asset->asset_id));
    if ($file) {
      return $file[(string) $asset->asset_id]['filename'];
    }

    return $default;
  }

  /**
   * Returns the description of the asset using the metadata.
   *
   * @param object $asset
   *   The asset in simplexmlelement or other object.
   * @param string $default
   *   Default value when empty (=== '').
   *
   * @return string
   *   The title or ''.
   *
   * @see get_metadata_name_title()
   * @see get_metadata_name_description()
   */
  public static function get_asset_description($asset, $default = '') {
    return self::get_asset_value($asset, self::get_metadata_name_description(), $default);
  }

  /**
   * Returns the value of an asset using it scope name ([set].[name]).
   *
   * Used now for retieving metadata values.
   *
   * @param object $asset
   *   The asset in simplexmlelement or other object.
   * @param string $field
   *   The name of the field to retrieve.
   * @param string $default
   *   The default value when value was not found or empty string (=== '').
   *
   * @return string
   *   The value or default.
   *
   * @see get_metadata_name_title()
   */
  public static function get_asset_value($asset, $field, $default = '') {
    // Inside XML result?
    if (isset($asset->items)) {
      return self::get_asset_value($asset->items->item, $field, $default);
    }

    if (is_array($asset->{$field})) {
      if (count($asset->{$field}) > 0) {
        return $asset->{$field}[0];
      }
      // the field is an array but does not contain any elements, return default
      return $default;
    }

    // Might be object that already has it set as [set].[name]
    if ((string) $asset->{$field} !== '') {
      return (string) $asset->{$field};
    }

    // See if field contains set.
    if (strpos($field, '.') !== FALSE) {
      // Need parts.
      list($set, $name) = explode('.', $field, 2);

      // Need metadata info.
      $metadata_fields = self::get_metadata_fields();

      // Field name is known metadata?
      if ($asset instanceof SimpleXMLElement && isset($metadata_fields[$set]) && !empty($metadata_fields[$set]['fields'][$name])) {
        $xpath = $metadata_fields[$set]['xpath'] . '/' . $name;
        $value = $asset->xpath($xpath);

        if ($value) {
          return (string) reset($value);
        }
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
  public static function lookup_user_with_name($name) {
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
  public static function precache_users_with_names(array $names) {
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
   * @param string $mail
   *   The mail address of the Drupal user.
   *
   * @return array
   *   The user array.
   */
  public static function lookup_user_with_mail($mail) {
    // Precache.
    self::precache_users_with_mails(array($mail));

    // Get the users.
    $users_mails = &drupal_static('mediamosa_ck_cache_drupal_users_mails', array());

    // Get the user from the cache.
    return isset($users_mails[$mail]) ? $users_mails[$mail] : NULL;
  }

  /**
   * Lookup the Drupal users using his name.
   *
   * Although I would like to use user_load_by_name(), I better use something
   * we can pre-cache when a lot of users need to be looked up. Also we only
   * need mail, uid and name from the users table.
   *
   * @param string $mails
   *   The mail address of the Drupal users to precache.
   */
  public static function precache_users_with_mails(array $mails) {
    // Store static. Storing it twice does not cost lost more memory, as the
    // users are objects, and these are referenced.
    $users_names = &drupal_static('mediamosa_ck_cache_drupal_users_names', array());
    $users_mails = &drupal_static('mediamosa_ck_cache_drupal_users_mails', array());
    $query = NULL;

    // Now see who we miss.
    foreach ($mails as $mail) {
      if (!isset($users_mails[$mail])) {
        if (!isset($query)) {
          $query = db_select('users', 'u')->fields(
            'u',
            array('mail', 'uid', 'name')
          );
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

  /**
   * Process the XML response (assets) for UI.
   *
   * @param mediamosa_connector_response $response
   *   The XML response.
   *
   * @return array
   *   An associtive array;
   *   'assets' => array(
   *     asset_id => array(
   *       - 'title'
   *         The title of the asset.
   *       - 'videotimestampmodified'
   *         Last modified date.
   *     );
   *   'item_count_total' => #, (number of total items in result)
   *   );
   */
  public static function process_assets_for_ui($response) {

    $processed_assets = array(
      'assets' => array(),
      'item_count_total' => 0,
    );

    // When FALSE, then something went wrong.
    if ($response !== FALSE) {
      foreach ($response->get()->xpath('items/item') as $item) {
        $asset_id = (string) $item->asset_id;
        $processed_assets['assets'][$asset_id] = array(
          'title' => mediamosa_ck::get_asset_title($item),
          'videotimestampmodified' => (string) $item->videotimestampmodified,
          'isprivate' => (string) $item->isprivate,
        );
      }

      $processed_assets['item_count_total'] = $response->get_header_item_count_total();
    }

    return $processed_assets;
  }

  /**
   * Search for assets, usage for UI.
   *
   * Does complex searches, except the data returned is processed and used for
   * UI, like tables. Idea behind this function is to force all table asset
   * output to have the same data formats for title and dates.
   *
   * Might be replaced by view in future.
   *
   * @return array
   *   An associative array;
   *   'assets' => array(
   *     asset_id => array(
   *       - 'title'
   *         The title of the asset.
   *       - 'videotimestampmodified'
   *         Last modified date.
   *     );
   *    ''
   *   );
   */
  public static function search_asset_for_ui(array $options = array()) {
    // Get the connector.
    $mediamosa_connector = mediamosa_ck::get_connector();

    // Setup default values.
    $options += array(
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 25,
      // Cql search query. Do not provide sortBy in query(!), use order_by and
      // order_direction.
      'cql' => '',
      // Order/sort by field.
      'order_by' => 'videotimestampmodified',
      // Direction either desc or asc.
      'order_direction' => 'desc',
    );

    // Do the request.
    return self::process_assets_for_ui($mediamosa_connector->search_asset($options));
  }

  /**
   * Get specific assets, usage for UI.
   *
   * Might be replaced by view in future.
   *
   * @return array
   *   An associtive array (or FALSE when failure);
   *   'assets' => array(
   *     [asset_id] => array(
   *       - 'title'
   *         The title of the asset.
   *       - 'videotimestampmodified'
   *         Last modified date.
   *     );
   *   );
   */
  public static function get_assets_for_ui(array $asset_ids) {
    // No asset IDs?
    if (empty($asset_ids)) {
      return array();
    }

    // Get the connector.
    $mediamosa_connector = mediamosa_ck::get_connector();

    // Setup default values.
    $options = array(
      // Order/sort by field.
      'order_by' => 'videotimestampmodified',
      // Direction either desc or asc.
      'order_direction' => 'desc',
    );

    // Collect them first.
    $assets = self::process_assets_for_ui($mediamosa_connector->get_assets($asset_ids, $options));
    if ($assets === FALSE) {
      return FALSE;
    }

    // The call does not garantee its in the same order, lets fix that.
    foreach ($asset_ids as $asset_id) {
      // Might not be returned (or found).
      if (isset($assets['assets'][$asset_id])) {
        $tmp = $assets['assets'][$asset_id];
        unset($assets['assets'][$asset_id]);
        $assets['assets'][$asset_id] = $tmp;
      }
    }

    return $assets;
  }

  /**
   * Split up the file managed uri into its parts.
   *
   * @param string $uri
   *   The url to split.
   *
   * @return array
   *   An associative array or FALSE when not MediaMosa asset filemanaged uri.
   *
   */
  public static function fileuri2parts($uri) {
    // Parse the url.
    $matches = array();
    if (!preg_match('@^mediamosa://asset/(?P<asset_id>[a-zA-Z0-9]+)/(?P<mediafile_id>[a-zA-Z0-9]+)/(?P<filename>[^/]+)$@i', $uri, $matches)) {
      return FALSE;
    }

    // Get the values.
    return array(
      'asset_id' => $matches['asset_id'],
      'mediafile_id' => $matches['mediafile_id'],
      'filename' => $matches['filename'],
    );
  }

  /**
   * Figure out the fid from an asset.
   *
   * @param string $asset_id
   *   The id of the asset
   *
   * @return integer
   *   The fid in Drupal or FALSE.
   */
  static public function get_fid_from_asset($asset_id) {
    $file = self::get_filemanaged_from_assets(array($asset_id));
    return empty($file) ? 0 : $file['fid'];
  }

  /**
   * Lookup the filemanaged data for the assets.
   *
   * @param array $asset_ids
   *   The asset ID to lookup.
   *
   * @return array
   *   An associative array where key is asset_id and value the fid.
   *   $asset_id => array(
   *     'fid' => $fid,
   *     'mediafile_id' => $mediafile_id,
   *     'filename' => $filename,
   *   );
   */
  static public function get_filemanaged_from_assets(array $asset_ids) {
    // Some cache.
    $assetids_to_fids = &drupal_static(__FUNCTION__, array());

    $result = array();

    // Look for ones we already have in cache.
    foreach ($asset_ids as $key => $asset_id) {
      if (isset($assetids_to_fids[$asset_id])) {
        $result[$asset_id] = $assetids_to_fids[$asset_id];
        unset($asset_ids[$key]);
      }
    }

    // Now find the rest.
    while (!empty($asset_ids)) {
      // Don't lookup more than this in one query.
      $max = 25;

      // Our query object.
      $query = db_select('file_managed', 'fm')
        ->fields('fm', array('fid', 'uri'));

      // Now add conditions.
      $db_or = db_or();
      foreach ($asset_ids as $key => $asset_id) {
        $db_or->condition('uri', 'mediamosa://asset/' . db_like($asset_id) .  '/%', 'LIKE');
        unset($asset_ids[$key]);

        // We reach max, stop.
        if (!$max--) {
          break;
        }
      }
      // Add or condition.
      $query->condition($db_or);

      // Now collect.
      foreach ($query->execute() as $file) {
        $parts = self::fileuri2parts($file->uri);
        if (!empty($parts)) {
          $result[$parts['asset_id']] = $assetids_to_fids[$parts['asset_id']] = array(
            'fid' => $file->fid,
            'mediafile_id' => $parts['mediafile_id'],
            'filename' => $parts['filename'],
          );

          // Keep memory in check.
          if (count($assetids_to_fids) > 1000) {
            array_shift($assetids_to_fids);
          }
        }
      }
    }

    // Done.
    return $result;
  }

  /**
   * Generates a token based on $value, the user session, and the private key.
   *
   * @param $value
   *   An additional value to base the token on.
   */
  public static function get_token($value = '') {
    return drupal_hmac_base64($value, drupal_get_private_key());
  }

  /**
   * Validates a token based on $value, the user session, and the private key.
   *
   * @param $token
   *   The token to be validated.
   * @param $value
   *   An additional value to base the token on.
   *
   * @return
   *   True for a valid token, false for an invalid token.
   */
  public static function valid_token($token, $value = '') {
    return $token == self::get_token($value);
  }

  /**
   * Trigger the url and do not wait for result.
   *
   * The url will be triggered, but function will return without waiting for the
   * result.
   *
   * @param string $url
   *   The URL to trigger.
   * @param array $options
   *   The option array. See drupal_http_request for options.
   */
  public static function trigger_http_request($url, array $options = array()) {
    // We will not wait for the result.
    $options['timeout'] = 1.0;

    // Do request.
    drupal_http_request($url, $options);
  }
}
