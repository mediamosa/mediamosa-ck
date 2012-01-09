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
 * Extends base class with supplying Media needs.
 */
class MediaMosaConnectorMediaWrapper extends MediaMosaCkConnectorWrapper {

  /**
   * Check for assets existence.
   *
   * @param array $asset_ids
   *
   * @return array with existing IDs, subset of input array.
   */
  static public function request_check_assets_existense($asset_ids) {

    $options = array(
      'return_asset_ids' => 'TRUE',
      'asset_id' => $asset_ids,
      'asset_id_match' => 'exact',
    );

    try {
      // Do request.
      $result = mediamosa_ck::request_get_fatal('asset', array('data' => $options));
    }
    catch (Exception $e) {
      drupal_set_message('Unable to check: ' . $e->getMessage(), 'error');
      return FALSE;
    }

    return self::get_asset_ids_array($result->xml);

  }

  /**
   * Parse to store in resulting array only 'asset_id'.
   *
   * @param xml_response
   *   The simplexmlelement object holding the result of the response.
   *
   * @return array with asset IDs.
   */
  static public function get_asset_ids_array($xml_response) {
    $items = array();
    if (isset($xml_response->items) && isset($xml_response->items->item)) {
      foreach ($xml_response->items->item as $obj_item) {
        $items[] = (string)$obj_item->asset_id;
      }
    }
    return $items;
  }

  /**
   * Get the recently changed/added assets.
   *
   * @deprecated
   *   Uses deprecated function mediamosa_connector::response2array, will
   *   be removed some. Will return simplexmlelement in future instead of
   *   array.
   *
   * @param $time_last
   *  Timestamp of the last time we asked for assets.
   * @param $limit
   *
   * @return $assets
   */
  static public function get_current_assets($time_last = 0, $offset = 0, $limit = 100) {
    // We used 'changed' parameter here before, but at the moment
    // 'videotimestampmodified' is much better option. Despite of its name, it
    // reflects also cases when we change only metadata.
    $cql = array();

    // Only when provided.
    if ($time_last) {
      $cql[] = strtr('videotimestampmodified >= @time_last', array('@time_last' => gmdate('Y-m-d\TH:i:s', $time_last)));
    }

    // Always sortby videotimestampmodified.
    $cql[] = 'sortby videotimestampmodified/ascending';

    $data = array(
      'cql' => implode(' ', $cql),
      'limit' => $limit,
      'offset' => $offset,
      // We don't sync assets that don't have mediafiles.
      'hide_empty_assets' => 'TRUE',
      // Keep it as fast as possible, no need to know.
      'calculate_total_count' => 'FALSE',
    );
    try {
      $response = mediamosa_ck::request_get_fatal('asset', array('data' => $data));
    }
    catch (Exception $e) {
      drupal_set_message(t('Error getting assets, @error.', array('@error' => $e->getMessage())), 'error');
      MediaMediaMosa::watchdog('Error getting assets (time_last = @time_last). Message: @error.', array('@time_last' => format_date($time_last), '@error' => $e->getMessage()), WATCHDOG_ERROR);
      return FALSE;
    }

    // Get the XML and convert it into an array.
    // @deprecated.
    $xml = mediamosa_connector::response2array($response);

    // Get the assets.
    return !empty($xml['items']['item']) ? $xml['items']['item'] : array();
  }

  /**
   * Get mediafiles of asset.
   *
   * @param $asset_id
   *
   * @return array $mediafiles
   */
  static public function get_asset_mediafiles($asset_id, $options = array()) {

    $request = strtr('asset/@asset_id/mediafile', array('@asset_id' => $asset_id));

    try {
      $response = mediamosa_ck::request_get_fatal($request);
    }
    catch (Exception $e) {
      MediaMediaMosa::watchdog('Error getting mediafiles for asset @asset_id.', array('@asset_id' => $asset_id), WATCHDOG_ERROR);

      // When set to fatal, rethrow.
      if ($options['fatal']) {
        throw $e;
      }

      return FALSE;
    }

    $xml = mediamosa_connector::response2array($response);

    $mediafiles = !empty($xml['items']['item']) ? $xml['items']['item'] : array();

    return $mediafiles;
  }

  /**
   * Return the number of assets in MediaMosa.
   */
  static public function get_number_of_assets() {
    $request = 'asset';

    try {
      $response = mediamosa_ck::request_get_fatal('asset', array('limit' => 1));
    }
    catch (Exception $e) {
      MediaMediaMosa::watchdog('Error getting assets.', array(), WATCHDOG_ERROR);
      return FALSE;
    }

    // Return # of assets.
    return (int) $response->xml->header->item_count_total;
  }

  /**
   * Return the highest videotimestampmodified.
   */
  static public function get_highest_videotimestamp_modified() {
    $request = 'asset';
    $cql = 'sortby videotimestampmodified/descending';

    try {
      $response = mediamosa_ck::request_get_fatal('asset', array('limit' => 1, 'cql' => $cql));
    }
    catch (Exception $e) {
      MediaMediaMosa::watchdog('Error getting assets.', array(), WATCHDOG_ERROR);
      return FALSE;
    }

    if (empty($response->xml->items->item)) {
      return FALSE;
    }

    $asset = $response->xml->items->item[0];
    return (string) $asset->videotimestampmodified;
  }
}
