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

class MediaMosaCkConnectorStatsWrapper extends MediaMosaCkConnectorWrapper {

  /**
   * Give me the assets/mediafiles that are most popular.
   *
   * @param array $options
   */
  static public function get_stats_popular_mediafiles($options) {

    $options += array(
      'limit' => 50,
      'offset' => 0,
    );

    // Setup query.
    $data = array('limit' => $options['limit'], 'offset' => $options['offset']);

    // What to return.
    $popular_mediafiles = array();

    try {
      // Do request.
      $response = mediamosa_ck::request_get_fatal('statistics/popularmediafiles', array('data' => $data));

      // Empty 1st.
      $popular_mediafiles = array(
        'mediafile_ids' => array(),
        'asset_ids' => array(),
      );

      // Collect the asset_ids.
      foreach ($response->xml->items->item as $item) {
        $popular_mediafiles['asset_ids'][] = (string) $item->asset_id;
        $popular_mediafiles['mediafile_ids'][(string) $item->asset_id] = (string) $item->mediafile_id;
      }

      // Get me the assets.
      $data = array('asset_id' => $popular_mediafiles['asset_ids'], 'asset_id_match' => 'exact', 'operator' => 'or');

      // Get the assets.
      $popular_mediafiles['response'] = mediamosa_ck::request_get_fatal('asset', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve the most popular mediafiles; @message.', array('@message' => $e->getMessage()));
      return FALSE;
    }

    return $popular_mediafiles;
  }

  /**
   * Get the latest mediafiles / videos.
   *
   * @param $options
   */
  static public function request_stats_latest_mediafiles($options = array()) {

    // Set default.
    $options += array(
      'limit' => 50,
      'offset' => 0,
    );

    // Setup query.
    $data = array(
      'limit' => $options['limit'],
      'offset' => $options['offset'],
      'order_by'=> 'videotimestamp',
      'order_direction'=> 'desc',
    );

    try {
      // Get the assets.
      return mediamosa_ck::request_get_fatal('asset', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve the latest mediafiles; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Get data usage statistics.
   *
   * @param string $type
   *   Value must be either "container" or "group" or "user".
   * @param integer $month
   * @param integer $year
   * @param array $data
   *   - limit
   *     Max items.
   *   - offset
   *     Start position.
   *   - order_by
   *     Column to sort, the TEXT in column is expected here.
   *   - order_direction
   *     ASC or DESC
   */
  static public function request_stats_datausage_video($type, $month, $year, $data = array()) {
    // Data.
    $data['type'] = $type;
    $data['month'] = $month;
    $data['year'] = $year;

    try {
      // Do request.
      return mediamosa_ck::request_get_fatal('statistics/datausagevideo', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve the most datausage videos; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Get newest mediafiles list.
   *
   * @return FALSE on failure, XML with data otherwise
   */
  static public function request_stats_newest_mediafiles($data = array()) {
    try {
      // Do request.
      return mediamosa_ck::request_get_fatal('statistics/newestmediafiles', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve the newest mediafiles; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }
}