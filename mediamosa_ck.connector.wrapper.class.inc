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
      if ($in[$param] === 'TRUE' || $in[$param] === 'FALSE') {
        continue; // not an number, ignore, expect TRUE/FALSE text.
      }

      $in[$param] = empty($in[$param]) ? 'FALSE' : 'TRUE';
    }

    return $in;
  }

  /**
   * Create a upload ticket and return the upload POST URL.
   */
  static public function create_upload_ticket(array $options = array()) {

    $options += array(
      'transcode_inherits_acl' => TRUE,
    );

    try {
      // Get the user_id for the owner.
      $user_id = mediamosa_ck::session_user_id();

      // Create the asset.
      $response = mediamosa_ck::request_post_fatal('asset/create', array('user_id' => $user_id));
      $data_create_mediafile['asset_id'] = (string) $response->xml->items->item->asset_id;

      // Downloadable setting.
      // Off until the is_empty_asset bug is fixed in MediaMosa.
      //$data_create_mediafile['is_downloadable'] = mediamosa_ck::setting_mediafile_downloadable() ? 'TRUE' : 'FALSE';

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

      // Inherit ACL.
      if ($options['transcode_inherits_acl']) {
        // Rebuild url.
        $parse_url = parse_url($action);

        // Split up query_string.
        $parse_url['query'] = mediamosa_ck::parse_query(empty($parse_url['query']) ? '' : $parse_url['query']);

        // Set inherit acl.
        $parse_url['query']['transcode_inherits_acl'] = 'TRUE';

        // Set inherit acl.
        $parse_url['query']['create_still'] = 'TRUE';

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
   * @param $acl_user_ids
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
   */
  static public function get_mediafile($mediafile_id, array $options = array(), $reset = FALSE) {

    // Save results.
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
   *  - show_collections (Default FALSE).
   *    TRUE / FALSE to include collections.
   */
  static public function get_asset($asset_id, array $options = array()) {
    $options += array(
      'show_stills' => TRUE,
      'show_collections' => FALSE,
    );

    // Convert bool to strings.
    $options = self::bool2string($options, array('show_stills', 'show_collections'));

    return mediamosa_ck::request_get_fatal('asset/' . rawurlencode($asset_id), array('data' => $options))->xml;
  }

  /**
   * Get the IDs of all mediafiles in the asset.
   *
   * @param $asset_id
   *   The asset_id.
   *
   * @return
   *   Array with IDs of mediafiles.
   */
  static public function get_asset_mediafile_ids($asset_id, array $options = array()) {
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
          if (drupal_strtoupper($mediafile->is_original_file) != 'TRUE') {
            continue;
          }
        }

        $mediafile_ids[] = (string) $mediafile->mediafile_id;
      }
    }
    catch (Exception $e) {
    }

    return $mediafile_ids;
  }

  /**
   * Get the collection.
   *
   * @param string $coll_id
   *  The ID of the collection to get.
   * @param array $options
   *  - hide_empty_assets
   *    TRUE / FALSE include empty assets (have no mediafiles).
   */
  static public function get_collection($coll_id, array $options = array(), $reset = FALSE) {

    // Save results.
    $collections = &drupal_static(__CLASS__ . '::' . __FUNCTION__, array());

    if ($reset) {
      $collections = array();
    }
    elseif (empty($collections[$coll_id])) {
      $options += array(
        'hide_empty_assets' => TRUE,
      );

      // Convert bool to strings.
      $options = self::bool2string($options, array('hide_empty_assets'));

      try {
        // Get the collection.
        $collections[$coll_id] = mediamosa_ck::request_get_fatal('collection/' . rawurlencode($coll_id),  array('data' => $options))->xml;
      }
      catch (Exception $e) {
        mediamosa_ck::watchdog_error('Unable to find collection; !message.', array('!message' => $e->getMessage()));
        return FALSE;
      }
    }

    return $collections[$coll_id];
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
    );

    try {
      // If no user_id is present, then we need the owner from the asset_id. We
      // don't fix this by using the is_app_admin, because it will then allow to
      // play unapropriate videos.
      if (empty($user_id)) {
        $user_id = self::get_asset_owner($asset_id);
      }

      $options['user_id'] = $user_id;
      $options['mediafile_id'] = $mediafile_id;
      $response_type = $options['response'];

      if (!empty($play_links[$asset_id][$mediafile_id][$response_type])) {
        return $play_links[$asset_id][$mediafile_id][$response_type];
      }

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
   * Get job for mediafile.
   *
   * @param $mediafile_id
   *
   * @return $jobs
   */
  static public function get_job($mediafile_id, $user_id = NULL) {
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
   * @return $profiles
   */
  static public function get_transcode_profiles() {
    // Return the transcode profiles.
    $transcode_profiles = &drupal_static(__FUNCTION__);
    if (!isset($transcode_profiles)) {
      $data = array();

      try {
        // Do request.
        $transcode_profiles = mediamosa_ck::request_get_fatal('transcode/profile', array('data' => $data))->xml;
      }
      catch (Exception $e) {
        mediamosa_ck::watchdog_error('Unable to retrieve the transcode profiles; !message.', array('!message' => $e->getMessage()));
      }
    }

    // Return the transcode profiles.
    return $transcode_profiles;
  }

  /**
   * Update the metadata for given asset ID.
   *
   * @param $asset_id
   * @param $options
   */
  static public function update_metadata($asset_id, array $options) {

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
   *   The options for the collection.
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
      $response = mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id), array('data' => $options));

      // Return the collection ID.
      return TRUE;
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to update asset; !message.', array('!message' => $e->getMessage()));
      return FALSE;
    }
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
      // cql search query. Do not provide sortBy in query(!).
      'cql' => '',
      // Order/sort by field.
      'orderby' => 'videotimestampmodified',
      // Direction either descending or ascending.
      'direction' => 'ascending',
      // Search within these collection(s).
      'coll_id' => array(),
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

    try {
      $response = mediamosa_ck::request_get_fatal('asset', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Error search assets (time_last = @time_last). Message: !message.', array('@time_last' => format_date($time_last), '!message' => $e->getMessage()), WATCHDOG_ERROR);
      return FALSE;
    }

    // Get the assets.
    return !empty($response) ? $response : array();
  }

  /**
   * Search collections. Uses the /collection REST call.
   *
   * @param $options
   *   Array with options for search.
   *
   * @return $assets
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
      mediamosa_ck::watchdog_error('Unable to create collection; !message.', array('!message' => $e->getMessage()));
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
}
