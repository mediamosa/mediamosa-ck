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
      if ($in[$param] === 'TRUE' || $in[$param] === 'FALSE') {
        continue; // not an number, ignore, expect TRUE/FALSE text.
      }

      $in[$param] = empty($in[$param]) ? 'FALSE' : 'TRUE';
    }

    return $in;
  }

  /**
   * Convert the MediaMosa variant of boolean string to boolean version.
   *
   * @param $str
   *   'TRUE', 'true', 'FALSE', 'false'.
   *
   * @return
   *   The boolean value.
   */
  static public function string2bool($str) {
    return (drupal_strtoupper($str) === 'TRUE' ? TRUE : FALSE);
  }

  /**
   * Create a upload ticket and return the upload POST URL.
   */
  static public function create_upload_ticket(array $options = array()) {

    $options += array(
      'transcode_inherits_acl' => TRUE,
      'create_still' => mediamosa_ck::setting_mediafile_upload_create_still(),
      'is_downloadable' => mediamosa_ck::setting_mediafile_downloadable(),
      'transcode' => array(),
    );

    try {
      // Get the user_id for the owner.
      $user_id = mediamosa_ck::session_user_id();

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('asset/create', array('user_id' => $user_id));
      $data_create_mediafile['asset_id'] = (string) $response->xml->items->item->asset_id;

      // Downloadable setting.
      // Off until the is_empty_asset bug is fixed in MediaMosa.
      //$data_create_mediafile['is_downloadable'] = $options['is_downloadable'] ? 'TRUE' : 'FALSE';

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
          // Set create still.
          $parse_url['query']['create_still'] = $options['create_still'] ? 'TRUE' : 'FALSE';
        }

        if (!empty($options['transcode'])) {
          // Set create still.
          $parse_url['query']['transcode'] = $options['transcode'];
        }

        // Rebuild.
        $parse_url['query'] = mediamosa_ck::http_build_query($parse_url['query']);

        // We are returning json.
        $action = mediamosa_ck::build_url($parse_url);
      }

      return array(
        'asset_id' => $data_create_mediafile['asset_id'],
        'mediafile_id' => $mediafile_id,
        'action' => $action,
      );
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to create upload ticket; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }
  }

  /**
   * Retrieve access limitations of mediafile.
   *
   * @param $mediafile_id
   * @param $user_id
   *
   * @return
   *   An array with domains and realms. Function is responsible to determine correct type for each element.
   */
  static public function get_access_control($mediafile_id, $user_id) {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    $data = array(
      'user_id' => $user_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id) . '/acl', array('data' => $data))->xml;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to get ACL rules; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Check if input is realm.
   */
  static public function is_realm($acl_domain_realm) {
    return ((strpos($acl_domain_realm, '@') === FALSE) ? FALSE : TRUE);
  }

  /**
   * Check if input is domain.
   */
  static public function is_domain($acl_domain_realm) {
    // If its a realm, its not a domain and visa versa.
    return !self::is_realm($acl_domain_realm) && !empty($acl_domain_realm);
  }

  /**
   * Apply access limitations to mediafile.
   *
   * @param $mediafile_id
   *   The ID of the mediafile.
   * @param $user_id
   *   The owner of the mediafile.
   * @param $acl_domain_realm
   *   A string with domains and realms. Function is responsible to determine
   *   correct type for each element.
   * @param $acl_user_id
   *   A string with users. This can be anything as MediaMosa does not care
   *   about the content of a user_id.
   */
  static public function set_access_control($mediafile_id, $user_id, $acl_domain_realm, $acl_user_id = '') {
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
   *  The ID of the mediafile to get.
   * @param array $options
   *  - show_stills (Default TRUE).
   *    TRUE / FALSE to include stills.
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  static public function get_mediafile($mediafile_id, array $options = array(), $cached = TRUE, $reset = FALSE) {

    // Set the default options for the REST call.
    $options += array(
      'show_stills' => TRUE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('show_stills'));

    // Get the mediafile.
    return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id), $options, FALSE, $cached, $reset)->xml;
  }

  /**
   * Get the asset. By default is this call cached(!).
   *
   * @param string $asset_id
   *  ID of the asset to get.
   * @param $options
   *  - show_stills (Default TRUE).
   *    TRUE / FALSE to include stills.
   *  - show_collections (Default FALSE).
   *    TRUE / FALSE to include collections.
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  static public function get_asset($asset_id, array $options = array(), $cached = TRUE, $reset = FALSE) {
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
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('show_stills', 'show_collections'));

      // Build ACL for alter.
    $acl = array(
      'acl_realm' => $options['acl_realm'],
      'acl_domain' => $options['acl_domain'],
      'acl_group_id' => $options['acl_group_id'],
      'acl_user_id' => empty($options['user_id']) ? $options['acl_user_id'] : $options['user_id'],
    );
    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_acl', $acl);

    // Copy back.
    $options['acl_realm'] = $acl['acl_realm'];
    $options['acl_domain'] = $acl['acl_domain'];
    $options['acl_group_id'] = $acl['acl_group_id'];
    $options['acl_user_id'] = $acl['acl_user_id'];
    unset($options['user_id']);


    // Do the REST call.
    return mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id), array('data' => $options), FALSE, $cached, $reset)->xml;
  }

  /**
   * Returns a list of the transcode profiles that are used.
   *
   * @param $asset_id
   *   The asset ID.
   * @param $options
   *   Options (reserved).
   *
   * @return
   *   Array with info about the current used transcode profiles.
   */
  static public function get_asset_current_used_transcode_profiles($asset_id, array $options = array()) {
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

// off until jobs REST will give back transcode jobs with used profile ID.
//      try {
//        // Now get the (running) jobs.
//        $xml = self::get_asset_jobs($asset_id);
//      }
//      catch (Exception $e) {
//        mediamosa_ck::watchdog_error('Unable to get jobs; !message.', array('!message' => $e->getMessage()));
//      }

    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to find existing mediafiles.');
    }

    return $current_used_transcode_profiles;
  }

  /**
   * Get the IDs of all mediafiles in the asset.
   *
   * @param $asset_id
   *   The asset_id.
   * @param $options
   *   - get_original_only
   *     Only get for the original mediafile the ID(s).
   *
   * @return
   *   Array with IDs of mediafiles.
   */
  static public function get_asset_mediafile_ids($asset_id, array $options = array()) {
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
   * Return the mediafile IDs of the original(s).
   *
   * @param $asset_id
   *   The asset_id.
   * @param $options
   *   The options of the REST call.
   *
   * @return
   *   Array with IDs of mediafiles.
   */
  static public function get_asset_mediafile_ids_original($asset_id, array $options = array()) {
    // Options for the REST call.
    $options['get_original_only'] = TRUE;

    return self::get_asset_mediafile_ids($asset_id, $options);
  }

  /**
   * Get the collection.
   *
   * @param string $coll_id
   *  The ID of the collection to get.
   * @param array $options
   *  - hide_empty_assets
   *    TRUE / FALSE include empty assets (have no mediafiles).
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The result XML.
   */
  static public function get_collection($coll_id, array $options = array(), $cached = TRUE, $reset = FALSE) {

    $options += array(
      'hide_empty_assets' => TRUE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('hide_empty_assets'));

    try {
      // Get the collection.
      return mediamosa_ck::request_get_fatal('collection/' . rawurlencode($coll_id),  array('data' => $options), FALSE, $cached, $reset)->xml;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to find collection; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }
  }

  /**
   * Return with array of the collections.
   *
   * @todo:
   *   Limit is 200.
   */
  static public function get_collections_for_options($options = array()) {
    $options += array(
      'offset' => 0,
      'limit' => 200, // Get max allowed per rest call.
    );

    // Get it.
    $collections = array();
    $xml = self::search_collection($options);
    if ($xml) {
      foreach ($xml->xpath('items/item') as $collection) {
        $collections[(string) $collection->coll_id] = (string) check_plain($collection->title);
      }
    }

    return $collections;
  }

  /**
   * Get the owner of the asset.
   *
   * @param string $asset_id
   *  The asset to get.
   * @param array $options
   *  Reserved.
   */
  static public function get_asset_owner($asset_id, array $options = array()) {
    $xml = self::get_asset($asset_id);
    return empty($xml->items->item->owner_id) ? '' : (string) $xml->items->item->owner_id;
  }

  /**
   * Create a play link.
   *
   * @param $asset_id
   *   The asset ID.
   * @param $mediafile_id
   *   The mediafile ID.
   * @param $user_id
   *   The user ID of the watcher. Best is to use
   *   mediamosa_ck::session_user_id() to get current user_id.
   * @param $options
   *   Set of options for the REST call.
   *   - response
   *     plain, object, metafile, still, download.
   *
   */
  static public function get_play_link($asset_id, $mediafile_id = '', $user_id = '', array $options = array()) {

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
    );

    try {
      // If no user_id is present, then we need the owner from the asset_id. We
      // don't fix this by using the is_app_admin, because it will then allow to
      // play unappropriate videos.
      if (empty($user_id)) {
        $user_id = self::get_asset_owner($asset_id);
      }

      $options['user_id'] = $user_id;
      $options['mediafile_id'] = $mediafile_id;
      $response_type = $options['response'];

      if (!empty($play_links[$asset_id][$mediafile_id][$response_type])) {
        return $play_links[$asset_id][$mediafile_id][$response_type];
      }

      // Build ACL for alter.
      $acl = array(
        'acl_realm' => $options['acl_realm'],
        'acl_domain' => $options['acl_domain'],
        'acl_group_id' => $options['acl_group_id'],
        'acl_user_id' => '',
      );
      // Allow modules to modify the acl information.
      drupal_alter('mediamosa_acl', $acl);

      // Copy back.
      $options['acl_realm'] = $acl['acl_realm'];
      $options['acl_domain'] = $acl['acl_domain'];
      $options['acl_group_id'] = $acl['acl_group_id'];

      $xml = mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id) . '/play', array('data' => $options))->xml;

      $play_links[$asset_id][$mediafile_id][$response_type] = empty($xml->items->item->output) ? FALSE : (string) $xml->items->item->output;
      return $play_links[$asset_id][$mediafile_id][$response_type];
    }
    catch (Exception $e) {
      // Don't display error to user, just log.
      mediamosa_ck::watchdog('Unable to create play ticket; @message.', array('@message' => $e->getMessage()));
      return FALSE;
    }
  }

  /**
   * Get jobs of mediafile.
   *
   * @param $mediafile_id
   *   The mediafile ID.
   * @param $user_id
   *   The owner of the mediafile.
   *
   * @return $jobs
   */
  static public function get_asset_jobs($asset_id, $user_id = FALSE) {
    if (!$user_id) {
      $user_id = mediamosa_ck::session_user_id();
    }

    $data = array(
      'user_id' => $user_id,
    );

    return mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id) . '/joblist', array('data' => $data))->xml;
  }

  /**
   * Get jobs of mediafile.
   *
   * @param $mediafile_id
   *   The mediafile ID.
   * @param $user_id
   *   The owner of the mediafile.
   *
   * @return $jobs
   */
  static public function get_mediafile_jobs($mediafile_id, $user_id = FALSE) {
    if (!$user_id) {
      $mediafile = MediaMosaCkConnectorWrapper::get_mediafile($mediafile_id);
      $user_id = (string) $mediafile->items->item->owner_id;
    }

    $data = array(
      'user_id' => $user_id,
    );

    return mediamosa_ck::request_get_fatal('mediafile/' . rawurlencode($mediafile_id) . '/joblist', array('data' => $data))->xml;
  }

  /**
   * Get transcode profiles.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  static public function get_transcode_profiles() {
    // Return the transcode profiles.
    $transcode_profiles = &drupal_static(__FUNCTION__);
    if (!isset($transcode_profiles)) {
      $data = array();

      try {
        // Do request.
        $transcode_profiles = mediamosa_ck::request_get_fatal_cached('transcode/profile', array('data' => $data))->xml;
      }
      catch (Exception $e) {
        mediamosa_ck::watchdog_error('Unable to retrieve the transcode profiles; !message.', array('!message' => $e->getMessage()));
      }
    }

    // Return the transcode profiles.
    return $transcode_profiles;
  }

  /**
   * Return the default transcode profile set by CK. When not set, find the
   * MediaMosa default.
   *
   * @return
   *   The transcode profile ID.
   */
  static public function get_default_transcode_profile() {

    $transcode_profile_default = mediamosa_connector::variable_get('media_mediamosa_transcode_profile_default');
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
   * @param $options
   *   - refresh_title (default TRUE)
   *     Reload the titles from MediaMosa.
   *
   * @return
   *   The transcode profile ID.
   */
  static public function get_allowed_transcode_profiles(array $options = array()) {
    // Default options.
    $options += array(
      'refresh_title' => TRUE,
    );

    // Get the allowed transcode profile.
    $transcode_profiles_allowed = mediamosa_connector::variable_get('media_mediamosa_transcode_profiles_allowed', array());
    if (!empty($transcode_profiles_allowed) && !$options['refresh_title']) {
      return $transcode_profiles_allowed;
    }

    // Get the profiles.
    $profiles = MediaMosaCkConnectorTranscodeWrapper::get_transcode_profiles();
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
      // Get the profiles.
      foreach ($profiles->items->item as $profile_id => $profile) {
        // Reset the title.
        if (!empty($transcode_profiles_allowed[(int) $profile->profile_id])) {
          $transcode_profiles_allowed[(int) $profile->profile_id] = t((string) $profile->profile);
        }
      }
    }

    // Store them for later retrieval.
    if (!empty($transcode_profiles_allowed)) {
      mediamosa_connector::variable_set('media_mediamosa_transcode_profiles_allowed', $transcode_profiles_allowed);
    }

    return $transcode_profiles_allowed;
  }

  /**
   * Create transcoded mediafile.
   *
   * @param $mediafile_id
   *   The mediafile ID of the original.
   * @param $user_id
   *   The owner of the asset.
   * @param $options
   *   The options for the REST call.
   *
   * @return
   *   Returns FALSE for failure.
   */
  static public function create_transcode($mediafile_id, $options = array()) {
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
   * @param $asset_id
   *   The ID of the asset.
   * @param $options
   *   The options for the REST call.
   */
  static public function update_metadata($asset_id, array $options = array()) {

    // Add action.
    $options += array(
      'user_id' => mediamosa_ck::session_user_id(),
      'action' => 'update'
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
   * @param $options
   *   The options for the asset.
   *
   * @return type
   */
  static public function update_asset($asset_id, array $options = array()) {
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
   * @param $options
   *   The options for the mediafile.
   *
   * @return type
   */
  static public function update_mediafile($mediafile_id, array $options = array()) {
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
   * @param $asset_id
   * @param $user_id
   */
  static public function delete_asset($asset_id, $user_id) {
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
      mediamosa_ck::watchdog_error('Unable to delete asset; !message.', array('!message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Mediafile delete.
   *
   * @param $mediafile_id
   *   The ID of the mediafile.
   * @param $user_id
   *   The ID of the user, must be owner.
   *
   * @return
   *   Returns TRUE when delete was successful.
   */
  static public function delete_mediafile($mediafile_id, $user_id) {
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
   * @param $options
   *   Array with options for search.
   *
   * @return
   *   The assets contained in a mediamosa_connector_response object.
   */
  static public function search_asset(array $options = array()) {
    // Setup default values.
    $options += array(
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 10,
      // cql search query. Do not provide sortBy in query(!), use order_by and order_direction.
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
    );
    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_acl', $acl);

    // Copy back.
    $data['acl_realm'] = $acl['acl_realm'];
    $data['acl_domain'] = $acl['acl_domain'];
    $data['acl_group_id'] = $acl['acl_group_id'];
    $data['acl_user_id'] = $acl['acl_user_id'];

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
   * @param $options
   *   Array with options for search.
   *
   * @return SimpleXMLElement
   *   XML result
   */
  static public function search_collection(array $options = array()) {

    // Setup default values.
    $options += array(
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 10,
      // cql search query. Do not provide sortBy in query(!).
      'cql' => '',
      // Order/sort by field.
      'orderby' => 'changed',
      // Direction either descending or ascending.
      'direction' => 'descending',

      // acl_user_id.
      'acl_user_id' => '',
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
      // We don't sync assets that don't have mediafiles.
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
    );
    // Allow modules to modify the acl information.
    drupal_alter('mediamosa_acl', $acl);

    // Copy back (only acl_user_id is used).
    $data['user_id'] = $acl['acl_user_id'];
    unset($data['acl_user_id']);

    try {
      return mediamosa_ck::request_get_fatal('collection', array('data' => $data))->xml;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Error search collection, message: !message.', array('!message' => $e->getMessage()), WATCHDOG_ERROR);
      return FALSE;
    }
  }

  /**
   * Create a basic collection.
   *
   * @param $options
   *   The options for the collection.
   *   - title
   *     The title of the collection.
   *   - description
   *     The description of the collection.
   *
   * @return type
   */
  static public function create_collection($options = array()) {
    try {
      $options += array(
        'user_id' => mediamosa_ck::session_user_id(),
      );

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('collection/create', array('data' => $options));

      // Return the collection ID.
      return (string) $response->xml->items->item->coll_id;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to create collection; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }
  }

  /**
   * Update a collection.
   *
   * @param $options
   *   The options for the collection.
   *   - title
   *     The title of the collection.
   *   - description
   *     The description of the collection.
   *
   * @return type
   */
  static public function update_collection($coll_id, array $options = array()) {
    try {
      $options += array(
        'user_id' => mediamosa_ck::session_user_id(),
      );

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('collection/' . rawurlencode($coll_id), array('data' => $options));

      // Return the collection ID.
      return (string) $response->xml->items->item->coll_id;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to create collection; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }
  }

  /**
   * Get the asset IDs in the collection.
   *
   * @param $coll_id
   *   The collection ID.
   * @param $options
   *   The options.
   */
  static public function get_collection_asset_ids($coll_id, $options = array()) {
    // Setup default values.
    $options += array(
      // Search for this collection.
      'coll_id' => array($coll_id),
      // Position in result set.
      'offset' => 0,
      // Number of items in result set.
      'limit' => 10,
      // cql search query. Do not provide sortBy in query(!).
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

    // Always sortby videotimestampmodified.
    if (!empty($options['orderby'])) {
      $cql[] = 'sortby ' . $options['orderby'] . !empty($options['direction']) ? '/' . $options['direction'] : '';
    }

    // Rebuild cql.
    $options['cql'] = implode(' ', $cql);

    // Do the search.
    $response = self::search_asset($options);

    // Return the xml.
    return empty($response) ? FALSE : $response->xml;
  }

  /**
   * Remove the relation between collection and assets.
   *
   * @param $coll_id
   *   The collection ID.
   * @param $user_id
   *   The user ID.
   * @param $asset_ids
   *   Array with asset_ids to remove from collection.
   */
  static public function delete_collection_asset_relations($coll_id, $user_id, array $asset_ids, array $options = array()) {
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
    return $response->code == 200 && $response->xml->header->request_result_id == 601;
  }

  /**
   * Add one or assets to the collection.
   *
   * @param $coll_id
   *   The collection ID.
   * @param $user_id
   *   The owner of the collection.
   * @param $asset_ids
   *   Array of asset_ids to put into collection.
   */
  static public function create_collection_asset_relations($coll_id, $user_id, array $asset_ids, array $options = array()) {
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
    return $response->code == 200 && $response->xml->header->request_result_id == 601;
  }

  /**
   * Delete a collection.
   *
   * @param $coll_id
   *   The ID of the collection.
   * @param $user_id
   *   The ID of the user, must be owner.
   * @param $options
   *   Array of options.
   *
   * @return
   *   Returns TRUE when delete was successful.
   */
  static public function delete_collection($coll_id, $user_id, $options = array()) {
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

    return $response->code == 200 && $response->xml->header->request_result_id == 601;
  }

  /**
   * Create a still for specific mediafile.
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
  static public function create_mediafile_still($mediafile_id, $user_id, $data = array()) {
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
   * @param $asset_id
   *   The asset ID.
   * @param $user_id
   *   The user ID.
   * @param $options
   *   The REST call options.
   */
  static public function delete_asset_still($asset_id, $user_id, array $options = array()) {
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
   * @param $options
   *   Reserverd
   * @param $cached
   *   Cache the result for REST call result.
   * @param $reset
   *   Reset the cache.
   *
   * @return SimpleXMLElement
   *   The XML response object.
   */
  static public function get_user($user_id, array $options = array(), $cached = TRUE, $reset = FALSE) {

    // Do the REST call.
    return mediamosa_ck::request_get_fatal('user/' . rawurlencode($user_id), array(), FALSE, $cached, $reset)->xml;
  }
}
