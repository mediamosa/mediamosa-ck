<?php
/**
 * @file
 * Our extended class on top of mediamosa_connector.
 */

class MediaMosaCkConnectorWrapper extends mediamosa_connector {

  // ------------------------------------------------------------------- Consts.
  // Transcode statuses used with get_asset_current_used_transcode_profiles().
  const TRANSCODE_STATUS_FINISHED = 'FINISHED';

  // ---------------------------------------------------------------- Functions.
  /**
   * Check the connector settings.
   */
  public static function check_connection() {
    try {
      mediamosa_ck::request_get_fatal('version');
    }
    catch (Exception $e) {
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Build the connection ID using the connector.
   */
  public static function build_connection_id() {
    $connector = mediamosa_ck::get_connector();

    return $connector->get_connection_id();
  }

  /**
   * Converts boolean to string ('TRUE' / 'FALSE').
   *
   * @param array $in
   *   Array with values to convert. May have other than specified in params.
   * @param array $params
   *   The params to convert.
   *
   * @return array
   *   The converted values.
   */
  public static function bool2string(array $in, array $params = array()) {
    foreach ($params as $param) {
      if (!isset($in[$param])) {
        continue;
      }

      if ($in[$param] === 'TRUE' || $in[$param] === 'FALSE') {
        // Not a number, ignore, expect TRUE/FALSE text.
        continue;
      }

      $in[$param] = empty($in[$param]) ? 'FALSE' : 'TRUE';
    }

    return $in;
  }

  /**
   * Convert the MediaMosa variant of boolean string to boolean version.
   *
   * @param string $str
   *   'TRUE', 'true', 'FALSE', 'false'.
   *
   * @return boolean
   *   The boolean value.
   */
  public static function string2bool($str) {
    return (drupal_strtoupper($str) === 'TRUE' ? TRUE : FALSE);
  }

  /**
   * Create a upload ticket and return the upload POST URL.
   *
   * @return array
   *   Returns an associative array;
   *   - 'action'
   *     The upload URL for the file.
   *   - 'progress_id'
   *     Progress ID to use when getting upload progression from server.
   *   - 'asset_id'
   *     The asset ID of the upload.
   *   - 'mediafile_id'
   *     The mediafile ID of the upload.
   */
  public static function create_upload_ticket(array $options = array()) {

    // Set default settings, when missing.
    $options += array(
      'transcode_inherits_acl' => TRUE,
      'create_still' => mediamosa_ck::setting_mediafile_upload_create_still(),
      'is_downloadable' => mediamosa_ck::setting_mediafile_downloadable(),
      'isprivate' => mediamosa_ck::setting_mediafile_isprivate(),
    );

    $process_auto_transcode = TRUE;
    if (module_exists('mediamosa_ck_upload') && mediamosa_ck_upload::get_bagit_file_ext() && mediamosa_ck_upload::get_allow_bagit_upload() && !empty($options['filename'])) {
      $bagit_exts = mediamosa_ck_upload::get_bagit_file_ext(TRUE);
      foreach ($bagit_exts as $bagit_ext) {
        $bagit_ext = trim($bagit_ext, ". \t\n\r\0\x0B");
        $bagit_ext = '.' . $bagit_ext;
        if (drupal_substr($options['filename'], -drupal_strlen($bagit_ext)) == $bagit_ext) {
          $process_auto_transcode = FALSE;
          break;
        }
      }
    }

    if (!isset($options['transcode']) && $process_auto_transcode) {
      $options['transcode'] = array();
      foreach (mediamosa_connector::variable_get('mediamosa_ck_transcodes_autostart') as $value) {
        if ($value > 0) {
          $options['transcode'][] = $value;
        }
      }
    }

    unset($options['filename']);

    try {
      // Get the user_id for the owner.
      $user_id = mediamosa_ck::session_user_id();

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('asset/create', array('user_id' => $user_id));
      $asset_id = (string) $response->items->item->asset_id;
      $data_create_mediafile['asset_id'] = $asset_id;

      if (isset($options['isprivate'])) {
        if ($options['isprivate'] == 'TRUE') {
          // Update the isprivate field of the asset.
          self::update_asset($asset_id, array('user_id' => $user_id, 'isprivate' => 'TRUE'));
        }
      }

      // Downloadable setting.
      $data_create_mediafile['is_downloadable'] = $options['is_downloadable'] ? 'TRUE' : 'FALSE';

      // Get optional tag.
      $tag = mediamosa_ck::setting_mediafile_tag();
      if (!empty($tag)) {
        $data_create_mediafile['tag'] = $tag;
      }

      // Create the mediafile.
      $response = mediamosa_ck::request_post_fatal('mediafile/create', array('data' => $data_create_mediafile, 'user_id' => $user_id));
      $mediafile_id = (string) $response->items->item->mediafile_id;

      // Create the upload ticket.
      $response = mediamosa_ck::request_post_fatal('mediafile/' . $mediafile_id . '/uploadticket/create', array('user_id' => $user_id));

      // Get the Action url from the result.
      $action = (string) $response->items->item->action;

      // Get the Progress id from the result.
      $progress_id = (int) $response->items->item->progress_id;
      $uploadprogress_url = $response->items->item->uploadprogress_url;

      // Inherit ACL / Create still.
      if ($options['transcode_inherits_acl'] || $options['create_still'] || !empty($options['transcode'])) {
        // Rebuild url.
        $parse_url = parse_url($action);

        // Split up query_string.
        $parse_url['query'] = mediamosa_ck::parse_query(empty($parse_url['query']) ? '' : $parse_url['query']);

        if ($options['transcode_inherits_acl']) {
          // Set inherit acl.
          $parse_url['query']['transcode_inherits_acl'] = 'TRUE';
        }

        if ($options['create_still']) {
          $parse_url['query']['create_still'] = $options['create_still'] ? 'TRUE' : 'FALSE';
          $parse_url['query']['still_type'] = 'NORMAL';
          $parse_url['query']['still_per_mediafile'] = '5';
          $parse_url['query']['start_frame'] = '10';
          $parse_url['query']['end_frame'] = '90';
        }

        if (!empty($options['transcode'])) {
          $parse_url['query']['transcode'] = $options['transcode'];
        }

        // Rebuild into string.
        $parse_url['query'] = mediamosa_ck::http_build_query($parse_url['query']);

        // Rebuild into url.
        $action = mediamosa_ck::build_url($parse_url);
      }

      return array(
        'asset_id' => $data_create_mediafile['asset_id'],
        'mediafile_id' => $mediafile_id,
        'progress_id' => $progress_id,
        'uploadprogress_url' => $uploadprogress_url,
        'action' => $action,
      );
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to create upload ticket; !message.', array('!message' => $e->getMessage()));
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Retrieve access limitations of mediafile.
   *
   * @param string $mediafile_id
   *   Mediafile id.
   * @param string $user_id
   *   user id.
   *
   * @return array
   *   An array with domains and realms. This function is responsible to
   *   determine correct type for each element.
   */
  public static function get_access_control($mediafile_id, $user_id) {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    $data = array(
      'user_id' => $user_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id) . '/acl', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to get ACL rules; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Check if input is realm.
   */
  public static function is_realm($acl_domain_realm) {
    return ((strpos($acl_domain_realm, '@') === FALSE) ? FALSE : TRUE);
  }

  /**
   * Check if input is domain.
   */
  public static function is_domain($acl_domain_realm) {
    // If its a realm, its not a domain and visa versa.
    return !self::is_realm($acl_domain_realm) && !empty($acl_domain_realm);
  }

  /**
   * Apply access limitations to mediafile.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile.
   * @param string $user_id
   *   The owner of the mediafile.
   * @param string $acl_domain_realm
   *   A string with domains and realms. Function is responsible to determine
   *   correct type for each element.
   * @param string $acl_user_id
   *   A string with users. This can be anything as MediaMosa does not care
   *   about the content of a user_id.
   */
  public static function set_access_control($mediafile_id, $user_id, $acl_domain_realm, $acl_user_id = '') {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    // Delete all?
    $delete_all = TRUE;

    $data = array(
      'user_id' => $user_id,
      'replace' => 'TRUE',
    );

    $access_entries = array();
    if (!empty($acl_domain_realm)) {
      preg_match_all('/[^\s|,]+/', $acl_domain_realm, $match);
      $access_entries = $match[0];
    }

    foreach ($access_entries as $entry) {
      $entry = trim($entry);
      if ($entry === '') {
        continue;
      }

      if (self::is_realm($entry)) {
        $data['aut_realm'][] = $entry;
      }
      else {
        $data['aut_domain'][] = $entry;
      }
    }

    $access_entries = array();
    if (!empty($acl_user_id)) {
      preg_match_all('/[^\s|,]+/', $acl_user_id, $match);
      $access_entries = $match[0];
    }

    foreach ($access_entries as $entry) {
      $entry = trim($entry);
      if ($entry === '') {
        continue;
      }

      $data['aut_user_id'][] = trim($entry);
    }

    // Anything to set?
    $delete_all = empty($data['aut_realm']) && empty($data['aut_domain']) && empty($data['aut_user_id']);

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/acl' . ($delete_all ? '/delete' : ''), array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to set ACL rule, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Get the mediafile.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile to get.
   * @param array $options
   *   - show_stills (Default TRUE).
   *     TRUE / FALSE to include stills.
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  public static function get_mediafile($mediafile_id, array $options = array(), $cached = TRUE, $reset = FALSE) {

    // Set the default options for the REST call.
    $options += array(
      'show_stills' => TRUE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('show_stills'));

    // Get the mediafile.
    return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id), $options, FALSE, $cached, $reset);
  }

  /**
   * Get the asset. By default is this call cached(!).
   *
   * @param string $asset_id
   *   ID of the asset to get.
   * @param array $options
   *   - show_stills (Default TRUE).
   *     TRUE / FALSE to include stills.
   *   - show_collections (Default FALSE).
   *     TRUE / FALSE to include collections.
   *   - is_app_admin
   *     Super user (Default FALSE).
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return mediamosa_connector_response_asset
   *   The (XML) response object specificly for asset REST call.
   */
  public static function get_asset($asset_id, array $options = array(), $cached = TRUE, $reset = FALSE) {
    // The options for the REST call.
    $options += array(
      'show_stills' => TRUE,
      'show_collections' => FALSE,

      // acl_realm.
      'acl_realm' => '',
      // acl_domain.
      'acl_domain' => '',
      // acl_group_ids.
      'acl_group_id' => array(),
      // acl_user_id.
      'acl_user_id' => '',

      // Super admin.
      'is_app_admin' => FALSE,
    );

    // Build ACL for alter.
    $acl = array(
      'acl_realm' => $options['acl_realm'],
      'acl_domain' => $options['acl_domain'],
      'acl_group_id' => $options['acl_group_id'],
      'acl_user_id' => empty($options['user_id']) ? $options['acl_user_id'] : $options['user_id'],
      'is_app_admin' => $options['is_app_admin'],
    );

    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_ck_acl', $acl);

    // Copy back.
    $options['acl_realm'] = $acl['acl_realm'];
    $options['acl_domain'] = $acl['acl_domain'];
    $options['acl_group_id'] = $acl['acl_group_id'];
    $options['acl_user_id'] = $acl['acl_user_id'];
    $options['is_app_admin'] = $acl['is_app_admin'];
    unset($options['user_id']);

    // Convert bool to strings.
    $options = self::bool2string($options, array('is_app_admin', 'show_stills', 'show_collections'));

    // Do the REST call.
    return new mediamosa_connector_response_asset(mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id), array('data' => $options), FALSE, $cached, $reset));
  }

  /**
   * Get the requested assets.
   *
   * @param array $asset_ids
   *   The assets to get.
   * @param array $options
   *   - 'count_view'
   *     Specifing 'FALSE' will prevent increasing the view count on the assets.
   *     (Default FALSE)
   *   - show_stills (Default TRUE).
   *     TRUE / FALSE to include stills.
   *   - show_collections (Default FALSE).
   *     TRUE / FALSE to include collections.
   *   - is_app_admin
   *     Super user (Default FALSE).
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  public static function get_assets($asset_ids, array $options = array()) {
    // The options for the REST call.
    $options += array(
      'show_stills' => TRUE,
      'show_collections' => FALSE,
      'count_view' => TRUE,

      // acl_realm.
      'acl_realm' => '',
      // acl_domain.
      'acl_domain' => '',
      // acl_group_ids.
      'acl_group_id' => array(),
      // acl_user_id.
      'acl_user_id' => '',

      // Super admin.
      'is_app_admin' => FALSE,
    );

    // The assets to retrieve.
    $options['asset_id'] = $asset_ids;

    // Build ACL for alter.
    $acl = array(
      'acl_realm' => $options['acl_realm'],
      'acl_domain' => $options['acl_domain'],
      'acl_group_id' => $options['acl_group_id'],
      'acl_user_id' => empty($options['user_id']) ? $options['acl_user_id'] : $options['user_id'],
      'is_app_admin' => $options['is_app_admin'],
    );

    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_ck_acl', $acl);

    // Copy back.
    $options['acl_realm'] = $acl['acl_realm'];
    $options['acl_domain'] = $acl['acl_domain'];
    $options['acl_group_id'] = $acl['acl_group_id'];
    $options['acl_user_id'] = $acl['acl_user_id'];
    $options['is_app_admin'] = $acl['is_app_admin'];
    unset($options['user_id']);

    // Convert bool to strings.
    $options = self::bool2string($options, array('count_view', 'show_stills', 'show_collections', 'is_app_admin'));

    // Do the REST call.
    return mediamosa_ck::request_get_fatal('assets', array('data' => $options));
  }

  /**
   * Returns a list of the transcode profiles that are used.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param array $options
   *   Options (reserved).
   *
   * @return array
   *   Array with info about the current used transcode profiles.
   */
  public static function get_asset_current_used_transcode_profiles($asset_id, array $options = array()) {
    // Data.
    $current_used_transcode_profiles = array();

    try {
      // Get existing mediafiles.
      $asset = MediaMosaCkConnectorWrapper::get_asset($asset_id);

      // Loop through the mediafiles.
      foreach ($asset->xpath('items/item/mediafiles/mediafile') as $mediafile) {
        // Skip original.
        if (self::string2bool($mediafile->is_original_file)) {
          continue;
        }

        // Collect the transcodes, ignore mediafiles that are not created using
        // a transcode profile.
        if (!empty($mediafile->transcode_profile_id)) {
          $current_used_transcode_profiles[(int) $mediafile->transcode_profile_id] = array(
            'transcode_profile_id' => (int) $mediafile->transcode_profile_id,
            'status' => self::TRANSCODE_STATUS_FINISHED,
            'mediafile_id' => (string) $mediafile->mediafile_id,
            'owner_id' => (string) $mediafile->owner_id,
            'is_downloadable' => MediaMosaCkConnectorWrapper::string2bool($mediafile->is_downloadable),
          );
        }
      }

      /*
       *  Disabled code until jobs REST will give back transcode jobs with
       *  used profile ID.
       *
       *  try {
       *    // Now get the (running) jobs.
       *    $xml = self::get_asset_jobs($asset_id);
       *   }
       *   catch (Exception $e) {
       *     mediamosa_ck::watchdog_error('Unable to get jobs; !message.',
       *       array('!message' => $e->getMessage())
       *     );
       *   }
       */
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to find existing mediafiles.');
    }

    return $current_used_transcode_profiles;
  }

  /**
   * Get the IDs of all mediafiles in the asset.
   *
   * @param string $asset_id
   *   The asset_id.
   * @param array $options
   *   - get_original_only
   *     Only get for the original mediafile the ID(s).
   *
   * @return array
   *   Array with IDs of mediafiles.
   */
  public static function get_asset_mediafile_ids($asset_id, array $options = array()) {
    // Options for the REST call.
    $options += array(
      'get_original_only' => FALSE,
    );

    $mediafile_ids = array();

    try {
      // Get the assets.
      $asset = self::get_asset($asset_id, array('show_stills' => FALSE));

      foreach ($asset->xpath('items/item/mediafiles/mediafile') as $mediafile) {
        // Only original IDs?
        if ($options['get_original_only']) {
          if (!MediaMosaCkConnectorWrapper::string2bool($mediafile->is_original_file)) {
            continue;
          }
        }

        $mediafile_ids[] = (string) $mediafile->mediafile_id;
      }
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to get mediafile IDs; !message.', array('!message' => $e->getMessage()));
    }

    return $mediafile_ids;
  }

  /**
   * Return the first original mediafile ID.
   *
   * @param string $asset_id
   *   The asset_id.
   * @param array $options
   *   The options of the REST call.
   *
   * @return string
   *   Mediafile_id.
   */
  public static function get_asset_mediafile_id_first_original($asset_id, array $options = array()) {
    // Options for the REST call.
    $options['get_original_only'] = TRUE;
    $mfs = self::get_asset_mediafile_ids($asset_id, $options);
    return reset($mfs);
  }

  /**
   * Return the mediafile IDs of the original(s).
   *
   * @param string $asset_id
   *   The asset_id.
   * @param array $options
   *   The options of the REST call.
   *
   * @return array
   *   Array with IDs of mediafiles.
   */
  public static function get_asset_mediafile_ids_original($asset_id, array $options = array()) {
    // Options for the REST call.
    $options['get_original_only'] = TRUE;

    return self::get_asset_mediafile_ids($asset_id, $options);
  }

  /**
   * Get the collection.
   *
   * @param string $coll_id
   *   The ID of the collection to get.
   * @param array $options
   *   - hide_empty_assets
   *     TRUE / FALSE include empty assets (have no mediafiles).
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The result XML.
   */
  public static function get_collection($coll_id, array $options = array(), $cached = TRUE, $reset = FALSE) {

    $options += array(
      'hide_empty_assets' => TRUE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('hide_empty_assets'));

    try {
      // Get the collection.
      return mediamosa_ck::request_get_fatal('collection/' . rawurlencode($coll_id), array('data' => $options), FALSE, $cached, $reset);
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to find collection; !message.', array('!message' => $e->getMessage()));
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Return with array of the collections.
   *
   * @todo:
   *   Limit is 200.
   */
  public static function get_collections_for_options($options = array(), $collections = array(), $max_length = 100) {
    $options += array(
      'offset' => 0,
      // Get max allowed per rest call:
      'limit' => 200,
    );

    // Get it.
    $response = self::search_collection($options);
    if ($response) {
      foreach ($response->xpath('items/item') as $collection) {
        $collections[(string) $collection->coll_id] = (string) check_plain(substr($collection->title, 0, $max_length));
      }
    }

    return $collections;
  }

  /**
   * Get the owner of the asset.
   *
   * @param string $asset_id
   *   The asset to get.
   * @param array $options
   *   Reserved.
   */
  public static function get_asset_owner($asset_id, array $options = array()) {
    $response = self::get_asset($asset_id);
    return empty($response->items->item->owner_id) ? '' : (string) $response->items->item->owner_id;
  }

  /**
   * Helper function to optionally convert output to https.
   */
  protected static function http_to_https($output) {
    if (variable_get('mediamosa_ck_view_https', FALSE)) {
      return str_replace('http://', 'https://', $output);
    }
    return $output;
  }

  /**
   * Create a play link.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $user_id
   *   The user ID of the watcher. Best is to use
   *   mediamosa_ck::session_user_id() to get current user_id.
   * @param array $options
   *   Set of options for the REST call.
   *   - response
   *     plain, object, metafile, still, download.
   *   - is_app_admin
   *     Super user (Default FALSE).
   */
  public static function get_play_link($asset_id, $mediafile_id = '', $user_id = '', array $options = array()) {

    $play_links = &drupal_static(__CLASS__ . '::' . __FUNCTION__, array());

    // Setup default.
    $options += array(
      'response' => 'plain',

      // acl_realm.
      'acl_realm' => '',
      // acl_domain.
      'acl_domain' => '',
      // acl_group_ids.
      'acl_group_id' => array(),

      // Catch fatal errors?
      'fatal' => FALSE,

      // Super admin.
      'is_app_admin' => FALSE,
    );

    try {
      $response_type = $options['response'];

      if (!empty($play_links[$asset_id][$mediafile_id][$response_type])) {
        return $play_links[$asset_id][$mediafile_id][$response_type];
      }

      // Build ACL for alter.
      $acl = array(
        'acl_realm' => $options['acl_realm'],
        'acl_domain' => $options['acl_domain'],
        'acl_group_id' => $options['acl_group_id'],
        // Not used by play call.
        'acl_user_id' => $user_id,
        'is_app_admin' => $options['is_app_admin'],
      );
      // Allow modules to modify the acl information.
      drupal_alter('mediamosa_ck_acl', $acl);

      // Copy back.
      $options['acl_realm'] = $acl['acl_realm'];
      $options['acl_domain'] = $acl['acl_domain'];
      $options['acl_group_id'] = $acl['acl_group_id'];
      $options['user_id'] = $user_id;
      $options['is_app_admin'] = $acl['is_app_admin'];

      $options = self::bool2string($options, array('is_app_admin'));

      // The 'is_app_admin' does not work for /play in 3.0.
      if ($options['is_app_admin'] == 'TRUE') {
        $options['user_id'] = self::get_asset_owner($asset_id);
      }

      // If mediafile_id is given: only try this one, and throw exception if
      // needed.
      if (isset($mediafile_id) && ($mediafile_id != '')) {
        $options['mediafile_id'] = $mediafile_id;
        $temp_profile_id = isset($options['profile_id']) ? $options['profile_id'] : '';
        unset($options['profile_id']);
        $result = mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id) . '/play', array('data' => $options, 'method' => 'GET'));
        if ($result->xml->header->request_result_id == mediamosa_sdk::ERRORCODE_OKAY) {
          if (!empty($result->xml->items->item->output)) {
            return MediaMosaCkConnectorWrapper::http_to_https($result->xml->items->item->output);
          }
        }
      }
      // Try to optain best mediafile here.
      // If no response, try the default transcode profile id.
      if (!isset($options['profile_id'])) {
        // Get the default ck transcode profile id.
        $options['profile_id'] = MediaMosaCkConnectorWrapper::get_default_transcode_profile();
      }
      unset($options['mediafile_id']);
      $result = mediamosa_ck::request('asset/' . rawurlencode($asset_id) . '/play', array('data' => $options, 'method' => 'GET'));

      if ($result->xml->header->request_result_id == mediamosa_sdk::ERRORCODE_OKAY) {
        if (!empty($result->xml->items->item->output)) {
          return MediaMosaCkConnectorWrapper::http_to_https($result->xml->items->item->output);
        }
      }
      elseif ($result->xml->header->request_result_id == mediamosa_sdk::ERRORCODE_NOT_AUTHORIZED) {
        throw new Exception('Not authorized', mediamosa_sdk::ERRORCODE_NOT_AUTHORIZED);
      }
      // If no transcode profile available, take the original.
      $options['mediafile_id'] = self::get_asset_mediafile_id_first_original($asset_id);
      unset($options['profile_id']);
      $result = mediamosa_ck::request('asset/' . rawurlencode($asset_id) . '/play', array('data' => $options, 'method' => 'GET'));
      if ($result->xml->header->request_result_id == mediamosa_sdk::ERRORCODE_OKAY) {
        if (!empty($result->xml->items->item->output)) {
          return MediaMosaCkConnectorWrapper::http_to_https($result->xml->items->item->output);
        }
      }
      elseif ($result->xml->header->request_result_id == mediamosa_sdk::ERRORCODE_NOT_AUTHORIZED) {
        throw new Exception('Not authorized', mediamosa_sdk::ERRORCODE_NOT_AUTHORIZED);
      }

      // If that also fails, show the image of the asset.
      $options['response'] = 'still';
      $result = mediamosa_ck::request('asset/' . rawurlencode($asset_id) . '/play', array('data' => $options, 'method' => 'GET'));
      if ($result->xml->header->request_result_id == mediamosa_sdk::ERRORCODE_OKAY) {
        if (!empty($result->xml->items->item->output)) {
          return '<img src="' . MediaMosaCkConnectorWrapper::http_to_https($result->xml->items->item->output) . '" alt="No media viewer found.">';
        }
      }

      // @todo: if that also fails,
      // show a default image (check if transcode in progress?).
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog('Unable to create play ticket; @message.', array('@message' => $e->getMessage()));
      if ($options['fatal']) {
        throw $e;
      }
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Get jobs of an asset.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $user_id
   *   The owner of the mediafile.
   *
   * @return object
   *   jobs.
   */
  public static function get_asset_jobs($asset_id, $user_id = FALSE) {
    if (!$user_id) {
      $user_id = mediamosa_ck::session_user_id();
    }

    $data = array(
      'user_id' => $user_id,
    );

    return mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id) . '/joblist', array('data' => $data));
  }

  /**
   * Get jobs of mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $user_id
   *   The owner of the mediafile.
   *
   * @return object
   *   $jobs.
   */
  public static function get_mediafile_jobs($mediafile_id, $user_id = FALSE) {
    if (!$user_id) {
      $mediafile = MediaMosaCkConnectorWrapper::get_mediafile($mediafile_id);
      $user_id = (string) $mediafile->items->item->owner_id;
    }

    $data = array(
      'user_id' => $user_id,
    );

    return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id) . '/joblist', array('data' => $data));
  }

  /**
   * Get transcode profiles.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  public static function get_transcode_profiles() {
    // Return the transcode profiles.
    $transcode_profiles = &drupal_static(__FUNCTION__);
    if (!isset($transcode_profiles)) {
      $data = array();

      try {
        // Do request.
        $transcode_profiles = mediamosa_ck::request_get_fatal_cached('transcode/profile', array('data' => $data));
      }
      catch (Exception $e) {
        mediamosa_ck::watchdog_error('Unable to retrieve the transcode profiles; !message.', array('!message' => $e->getMessage()));
      }
    }

    // Return the transcode profiles.
    return $transcode_profiles;
  }

  /**
   * Return the default transcode profile set by CK.
   *
   * When not set, find the MediaMosa default.
   *
   * @return integer.
   *   The transcode profile ID.
   */
  public static function get_default_transcode_profile() {

    $transcode_profile_default = mediamosa_connector::variable_get('mediamosa_ck_transcode_profile_default');
    if (!$transcode_profile_default) {

      // Get the profiles.
      $profiles = self::get_transcode_profiles();
      if (!$profiles) {
        mediamosa_ck::watchdog_error(t('Unable to retrieve transcode profiles.'));
      }

      $transcode_profile_default = 0;
      if ($profiles && isset($profiles->items->item)) {
        foreach ($profiles->items->item as $profile_id => $profile) {
          // Store the first one as default.
          if (empty($transcode_profile_default)) {
            $transcode_profile_default = (int) $profile->profile_id;
          }

          // Set to default?
          if (self::string2bool((string) $profile->default)) {
            $transcode_profile_default = (int) $profile->profile_id;
            // FIFO.
            break;
          }
        }
      }
    }

    return $transcode_profile_default;
  }

  /**
   * Return the allowed transcode profiles.
   *
   * @param array $options
   *   - refresh_title (default TRUE)
   *     Reload the titles from MediaMosa.
   *
   * @return integer
   *   The transcode profile ID.
   */
  public static function get_allowed_transcode_profiles(array $options = array()) {
    // Default options.
    $options += array(
      'refresh_title' => TRUE,
    );

    // Get the allowed transcode profile.
    $transcode_profiles_allowed = mediamosa_connector::variable_get('mediamosa_ck_transcode_profiles_allowed', array());
    if (!empty($transcode_profiles_allowed) && !$options['refresh_title']) {
      return $transcode_profiles_allowed;
    }

    // Get the profiles.
    $profiles = MediaMosaCkConnectorWrapper::get_transcode_profiles();
    if (!$profiles) {
      mediamosa_ck::watchdog_error(t('Unable to retrieve transcode profiles.'));
    }

    // No allowed set? Then get them all and allow them all.
    if (empty($transcode_profiles_allowed)) {
      // Get the profiles.
      foreach ($profiles->items->item as $profile_id => $profile) {
        $transcode_profiles_allowed[(int) $profile->profile_id] = t((string) $profile->profile);
      }
    }
    else {
      // Mark them so we remove 'removed' transcode profiles.
      foreach (array_keys($transcode_profiles_allowed) as $key) {
        $transcode_profiles_allowed[$key] = FALSE;
      }

      // Get the profiles.
      foreach ($profiles->items->item as $profile_id => $profile) {
        if (isset($transcode_profiles_allowed[(int) $profile->profile_id])) {
          $transcode_profiles_allowed[(int) $profile->profile_id] = t((string) $profile->profile);
        }
      }

      // Any with FALSE value is no longer present in mediamosa.
      foreach (array_keys($transcode_profiles_allowed) as $key) {
        if ($transcode_profiles_allowed[$key] === FALSE) {
          unset($transcode_profiles_allowed[$key]);
        }
      }
    }

    // Store them for later retrieval.
    if (!empty($transcode_profiles_allowed)) {
      mediamosa_connector::variable_set('mediamosa_ck_transcode_profiles_allowed', $transcode_profiles_allowed);
    }

    return $transcode_profiles_allowed;
  }

  /**
   * Create transcoded mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile ID of the original.
   * @param array $options
   *   The options for the REST call.
   *
   * @return bool
   *   Returns FALSE for failure.
   */
  public static function create_transcode($mediafile_id, $options = array()) {
    // Check the parameters.
    if (empty($mediafile_id)) {
      return FALSE;
    }

    $options += array(
      'user_id' => mediamosa_ck::session_user_id(),
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/transcode', array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to start a transcode; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Update the metadata for given asset ID.
   *
   * @param string $asset_id
   *   The ID of the asset.
   * @param array $options
   *   The options for the REST call.
   */
  public static function update_metadata($asset_id, array $options = array()) {

    // Add action.
    $options += array(
      'user_id' => mediamosa_ck::session_user_id(),
      'action' => 'update',
    );

    try {
      // Do request.
      mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/metadata', array('data' => $options));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to update metadata; ' . $e->getMessage(), 'error');
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Update a asset.
   *
   * @param array $options
   *   The options for the asset.
   *
   * @return mediamosa_connector_response
   *   The mediamosa_connector_response object or FALSE when failed.
   */
  public static function update_asset($asset_id, array $options = array()) {
    try {
      $options += array(
        'user_id' => mediamosa_ck::session_user_id(),
      );

      // Convert bool to strings.
      $options = self::bool2string($options, array('isprivate'));

      // Create the asset.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id), array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to update asset; !message.', array('!message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Update a mediafile.
   *
   * @param string $mediafile_id
   *   id of the mediafile to update.
   * @param string $options
   *   The options for the mediafile.
   *
   * @return mediamosa_connector_response
   *   The mediamosa_connector_response object or FALSE when failed.
   */
  public static function update_mediafile($mediafile_id, array $options = array()) {
    try {
      $options += array(
        'user_id' => mediamosa_ck::session_user_id(),
      );

      // Convert bool to strings.
      $options = self::bool2string($options, array('is_downloadable', 'transcode_inherits_acl'));

      // Create the asset.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id), array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to update mediafile; !message.', array('!message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Asset delete.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $user_id
   *   The owner of the mediafile.
   *
   * @return mediamosa_connector_response
   *   The mediamosa_connector_response object or FALSE when failed.
   */
  public static function delete_asset($asset_id, $user_id) {
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
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/delete', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete asset; !message.', array('!message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Mediafile delete.
   *
   * @param string $mediafile_id
   *   id of the mediafile to update.
   * @param string $user_id
   *   The ID of the user, must be owner.
   *
   * @return boolean.
   *   Returns TRUE when delete was successful.
   */
  public static function delete_mediafile($mediafile_id, $user_id) {
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
      $response = mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/delete', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete mediafile; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }

    return $response->code == 200;
  }

  /**
   * Search assets. Uses the /asset REST call.
   *
   * @param array $options
   *   An associative array;
   *   - is_app_admin
   *     Super user (Default FALSE).
   *
   * @return mediamosa_connector_response
   *   The assets contained in a mediamosa_connector_response object.
   */
  public static function search_asset(array $options = array()) {
    // Setup default values.
    $options += array(
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 10,
      // Cql search query. Do not provide sortBy in query(!), use order_by and
      // order_direction.
      'cql' => '',
      // Order/sort by field.
      'order_by' => 'videotimestampmodified',
      // Direction either desc or asc.
      'order_direction' => 'asc',
      // Search within these collection(s).
      'coll_id' => array(),

      // acl_realm.
      'acl_realm' => '',
      // acl_domain.
      'acl_domain' => '',
      // acl_group_ids.
      'acl_group_id' => array(),
      // acl_user_id.
      'acl_user_id' => '',

      // Super admin.
      'is_app_admin' => FALSE,
    );

    // CQL search.
    $cql = empty($options['cql']) ? array() : array($options['cql']);

    // Always sortby videotimestampmodified.
    if (!empty($options['order_by'])) {
      $cql[] = 'sortby ' . $options['order_by'] . (!empty($options['order_direction']) ? '/' . ($options['order_direction'] == 'asc' ? 'ascending' : 'descending') : '');
    }
    // Skip these.
    unset($options['order_by']);
    unset($options['order_direction']);

    // Settings for REST call.
    $data = array(
      'cql' => implode(' ', $cql),
      'limit' => $options['limit'],
      'offset' => $options['offset'],
      // We don't sync assets that don't have mediafiles.
      'hide_empty_assets' => 'TRUE',
      // Get total count, need pager.
      'calculate_total_count' => 'TRUE',
    );

    // Add our options to $data.
    $data = array_merge($options, $data);

    // Build ACL for alter.
    $acl = array(
      'acl_realm' => $data['acl_realm'],
      'acl_domain' => $data['acl_domain'],
      'acl_group_id' => $data['acl_group_id'],
      'acl_user_id' => $data['acl_user_id'],
      'is_app_admin' => $data['is_app_admin'],
    );
    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_ck_acl', $acl);

    // Copy back.
    $data['acl_realm'] = $acl['acl_realm'];
    $data['acl_domain'] = $acl['acl_domain'];
    $data['acl_group_id'] = $acl['acl_group_id'];
    $data['acl_user_id'] = $acl['acl_user_id'];
    $data['is_app_admin'] = $acl['is_app_admin'];

    // Convert bool to string.
    $data = self::bool2string($data, array('is_app_admin'));

    try {
      return mediamosa_ck::request_get_fatal('asset', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Error search assets. Message: !message.', array('!message' => $e->getMessage()), WATCHDOG_ERROR);
    }

    // Got here, then return FALSE.
    return FALSE;
  }

  /**
   * Search collections. Uses the /collection REST call.
   *
   * @param array $options
   *   Array with options for search.
   *
   * @return SimpleXMLElement
   *   XML result
   */
  public static function search_collection(array $options = array()) {

    // Setup default values.
    $options += array(
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 10,
      // Cql search query. Do not provide sortBy in query(!).
      'cql' => '',
      // Order/sort by field.
      'orderby' => 'changed',
      // Direction either descending or ascending.
      'direction' => 'descending',

      // acl_user_id.
      'acl_user_id' => '',

      // Super admin.
      'is_app_admin' => FALSE,
    );

    // CQL search.
    $cql = empty($options['cql']) ? array() : array($options['cql']);

    // Always sortby videotimestampmodified.
    if (!empty($options['orderby'])) {
      $cql[] = 'sortby ' . $options['orderby'] . !empty($options['direction']) ? '/' . $options['direction'] : '';
    }

    // Settings for REST call.
    $data = array(
      'cql' => implode(' ', $cql),
      'limit' => $options['limit'],
      'offset' => $options['offset'],
      'hide_empty_assets' => 'TRUE',
      // Get total count, need pager.
      'calculate_total_count' => 'TRUE',
    );

    // Add our options to $data.
    $data = array_merge($data, $options);

    // Build ACL for alter.
    $acl = array(
      'acl_realm' => '',
      'acl_domain' => '',
      'acl_group_id' => '',
      'acl_user_id' => empty($data['user_id']) ? $data['acl_user_id'] : $data['user_id'],
      'is_app_admin' => $options['is_app_admin'],
    );
    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_ck_acl', $acl);

    // Copy back (only acl_user_id is used).
    $data['user_id'] = $acl['acl_user_id'];
    unset($data['acl_user_id']);

    // Convert bool to string.
    $data = self::bool2string($data, array('is_app_admin'));

    try {
      return mediamosa_ck::request_get_fatal('collection', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::set_message_exception($e, 'Error search collection');
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Create a basic collection.
   *
   * @param array $options
   *   The options for the collection.
   *   - title
   *     The title of the collection.
   *   - description
   *     The description of the collection.
   *
   * @return collection_id
   *   returns created collection id, or FALSE.
   */
  public static function create_collection($options = array()) {
    try {
      $options += array(
        'user_id' => mediamosa_ck::session_user_id(),
      );

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('collection/create', array('data' => $options));

      // Return the collection ID.
      return (string) $response->items->item->coll_id;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to create collection; !message.', array('!message' => $e->getMessage()));
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Update a collection.
   *
   * @param array $options
   *   The options for the collection.
   *   - title
   *     The title of the collection.
   *   - description
   *     The description of the collection.
   *
   * @return collection_id
   *   returns updated collection id, or FALSE.
   */
  public static function update_collection($coll_id, array $options = array()) {
    try {
      $options += array(
        'user_id' => mediamosa_ck::session_user_id(),
      );

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('collection/' . rawurlencode($coll_id), array('data' => $options));

      // Return the collection ID.
      return (string) $response->items->item->coll_id;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to create collection; !message.', array('!message' => $e->getMessage()));
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Get the asset IDs in the collection.
   *
   * @param string $coll_id
   *   The collection ID.
   * @param array $options
   *   The options.
   */
  public static function get_collection_asset_ids($coll_id, $options = array()) {
    // Setup default values.
    $options += array(
      // Search for this collection.
      'coll_id' => array($coll_id),
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 10,
      // Cql search query. Do not provide sortBy in query(!).
      'cql' => '',
      // Order/sort by field.
      'orderby' => 'changed',
      // Direction either descending or ascending.
      'direction' => 'descending',
      // Return only asset_ids.
      'return_asset_ids' => TRUE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('return_asset_ids'));

    // CQL search.
    $cql = empty($options['cql']) ? array() : array($options['cql']);

    // Rebuild cql.
    $options['cql'] = implode(' ', $cql);

    // Do the search.
    return self::search_asset($options);
  }

  /**
   * Remove the relation between collection and assets.
   *
   * @param string $coll_id
   *   The collection ID.
   * @param string $user_id
   *   The user ID.
   * @param array $asset_ids
   *   Array with asset_ids to remove from collection.
   */
  public static function delete_collection_asset_relations($coll_id, $user_id, array $asset_ids, array $options = array()) {
    // Options.
    $options = array(
      'user_id' => $user_id,
      'asset_id' => $asset_ids,
    );

    try {
      $response = mediamosa_ck::request_post_fatal('collection/' . rawurlencode($coll_id) . '/asset_relation/delete', array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete asset relation(s) with collection; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }

    // Is ok?
    return $response->header->request_result_id == 601;
  }

  /**
   * Add one or assets to the collection.
   *
   * @param string $coll_id
   *   The collection ID.
   * @param string  $user_id
   *   The owner of the collection.
   * @param array $asset_ids
   *   Array of asset_ids to put into collection.
   */
  public static function create_collection_asset_relations($coll_id, $user_id, array $asset_ids, array $options = array()) {
    // Options.
    $options = array(
      'user_id' => $user_id,
      'asset_id' => $asset_ids,
    );

    try {
      $response = mediamosa_ck::request_post_fatal('collection/' . rawurlencode($coll_id) . '/asset_relation', array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to add asset(s) to collection; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }

    // Is ok?
    return $response->header->request_result_id == 601;
  }

  /**
   * Delete a collection.
   *
   * @param string $coll_id
   *   The ID of the collection.
   * @param string $user_id
   *   The ID of the user, must be owner.
   * @param array $options
   *   Array of options.
   *
   * @return boolean
   *   Returns TRUE when delete was successful.
   */
  public static function delete_collection($coll_id, $user_id, $options = array()) {
    // Check the parameters.
    if (empty($coll_id) || empty($user_id)) {
      return FALSE;
    }

    $options += array(
      'user_id' => $user_id,
    );

    try {
      // Do request.
      $response = mediamosa_ck::request_post_fatal('collection/' . rawurlencode($coll_id) . '/delete', array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete collection; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }

    return $response->header->request_result_id == 601;
  }

  /**
   * Create a still for specific mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $user_id
   *   The owner of the mediafile.
   * @param array $data
   *   Array with all the still parameters.
   *
   * @return mediamosa_connector_response
   *   Returns FALSE when failure or mediamosa_connector_response.
   */
  public static function create_mediafile_still($mediafile_id, $user_id, $data = array()) {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    $data += array(
      'user_id' => $user_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/still/create', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to start a still process, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Delete still(s) from a asset.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $user_id
   *   The user ID.
   * @param array $options
   *   The REST call options.
   */
  public static function delete_asset_still($asset_id, $user_id, array $options = array()) {
    // Check the parameters.
    if (empty($asset_id)) {
      return FALSE;
    }

    $options += array(
      'user_id' => $user_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/still/delete', array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete stills, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Get the user. By default is this call cached(!).
   *
   * @param string $user_id
   *   ID of the user to get.
   * @param array $options
   *   Reservered
   * @param boolean $cached
   *   Cache the result for REST call result.
   * @param boolean $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  public static function get_user($user_id, array $options = array(), $cached = TRUE, $reset = FALSE) {
    // Do the REST call.
    return mediamosa_ck::request_get_fatal('user/' . rawurlencode($user_id), array(), FALSE, $cached, $reset);
  }

  /**
   * Get one or more applications metadata tag definitions.
   *
   * @param array $options
   *   An associative array;
   *   - 'name'
   *     Optional property name to search on.
   * 	 - 'offset'
   *     The starting items position in the result set.
   *   - 'limit'
   *     The maximum number of items to return.
   *   - 'order_by'
   *      On what column the result set should be ordered. Must be either
   *      "prop_id" or "prop_name" or "type" or "created" or "changed".
   *   - 'order_direction'
   *     The direction of the result set.
   *
   * @return mediamosa_connector_response
   *   The response object.
   */
  public static function get_metadata_tag($options = array()) {
    $options += array(
      'name' => '',
      'offset' => 0,
      'limit' => 10,
      'order_by' => '',
      'order_direction' => 'asc',
      'include_default_definitions' => TRUE,
    );

    $options = self::bool2string($options, array('include_default_definitions'));

    $data = array(
      'name' => $options['name'],
      'offset' => $options['offset'],
      'limit' => $options['limit'],
      'order_by' => $options['order_by'],
      'order_direction' => $options['order_direction'],
      'include_default_definitions' => $options['include_default_definitions'],
    );

    try {
      // Do request.
      return mediamosa_ck::request_get_fatal('metadata_tag', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve metadata tags, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * @param string $name
   *   The name of the property to create
   * @param string $type
   *   Either 'datetime', 'int', 'char'.
   * @param array $options
   *   An associative array.
   *   - 'is_hidden' boolean
   *     Either TRUE for hidden or FALSE. Hidden will hide the metadata on asset
   *     output.
   */
  public static function create_metadata_tag($name, $type, array $options = array()) {
    $options += array(
      'is_hidden' => FALSE,
    );

    $options = self::bool2string($options, array('is_hidden'));

    $data = array(
      'name' => $name,
      'type' => $type,
      'is_hidden' => $options['is_hidden'],
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('metadata_tag/create', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve metadata tags, reason; @message.', array('@message' => $e->getMessage()));
    }

    // Got here, return FALSE.
    return FALSE;
  }

  /**
   * Change ownership.
   *
   * @param string $owner_id
   *   The owner of the asset.
   * @param string $new_owner_id
   *   The options for the REST call.
   *
   * @return bool
   *   Returns FALSE for failure.
   */
  static public function change_ownership($owner_id, $new_owner_id) {

    $data = array(
      'old_owner_id' => $owner_id,
      'new_owner_id' => $new_owner_id,
    );
    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('change_ownership', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to transfer objects to new owner; @message.', array('@message' => $e->getMessage()));
    }

    // Got here, return FALSE.
    return FALSE;
  }
}
