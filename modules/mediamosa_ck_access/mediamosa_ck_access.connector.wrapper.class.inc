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

class MediaMosaCkConnectorAccessWrapper extends MediaMosaCkConnectorWrapper {
  /**
   * Retrieve access limitations of mediafile.
   *
   * @param $mediafile_id
   * @param $user_id
   *
   * @return array $mediafile_id
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
      drupal_set_message('Unable to get ACL rules, reason; ' . $e->getMessage(), 'error');
    }

    return FALSE;
  }

  /**
   * Check if input is realm.
   */
  static public function is_realm($domain_realm) {
    return ((strpos($domain_realm, '@') === FALSE) ? FALSE : TRUE);
  }

  /**
   * Check if input is domain.
   */
  static public function is_domain($domain_realm) {
    // If its a realm, its not a domain and visa versa.
    return !self::is_realm($domain_realm) && !empty($domain_realm);
  }

  /**
   * Apply access limitations to mediafile.
   *
   * @param $mediafile_id
   * @param $user_id
   * @param string $domain_realm
   *   An array with domains and realms. Function is responsible to determine correct type for each element.
   */
  static public function set_access_control($mediafile_id, $user_id, $domain_realm) {
    // Check the parameters.
    if (empty($mediafile_id) || empty($user_id)) {
      return FALSE;
    }

    $data = array(
      'user_id' => $user_id,
      'replace' => 'TRUE',
    );

    if ($domain_realm) {
      preg_match_all('/[^\s]+/', $domain_realm, $match);
      $access_entries = $match[0];
    }
    else {
      $access_entries = array();
    }

    foreach ($access_entries as $entry) {
      if (self::is_realm($entry)) {
        $data['aut_realm'][] = $entry;
      }
      else {
        $data['aut_domain'][] = $entry;
      }
    }

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('mediafile/' . rawurlencode($mediafile_id) . '/acl' . ($access_entries ? '' : '/delete'), array('data' => $data));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to set ACL rule, reason; ' . $e->getMessage(), 'error');
    }

    return FALSE;
  }
}
