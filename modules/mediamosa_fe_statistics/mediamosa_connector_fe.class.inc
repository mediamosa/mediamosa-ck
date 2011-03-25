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
 * Mediamosa connector FrontEnd is a simple expansion of the mediamosa_connector class.
 * Designed to use in front-end application, this class catching all exceptions, if any,
 * and just return FALSE in that case.
 */

require_once (drupal_get_path('module', 'mediamosa_connector') .'/mediamosa_connector.class.inc');

/**
 * mediamosa_connector_fe designed to use in FrontEnd apps.
 */
class mediamosa_connector_ex extends mediamosa_connector {
  /**
   * @param string $uri
   *  The REST uri.
   * @param array $options
   *   (optional) An array which can have one or more of following keys:
   *   - user_id (mediamosa)
   *       The user_id of the Mediamosa call. Default is empty.
   *   - group_id (mediamosa)
   *       The group_id of the Mediamosa call. Default is empty.
   *   - fatal
   *       Boolean value, TRUE makes the call throw an exception if the call
   *       failed. That means, failed to call, failed to access the rest etc.
   *       Empty results are not fatal. Default is FALSE.
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body.
   *       Or
   *       A array as name => value.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   * @return FALSE in case of exception or invalid result.
   *
   * @see mediamosa_connector
   */
  public function request($uri, array $options = array()) {

    // Exceptions are handled within this function.
    $options['fatal'] = TRUE;

    try {
      $response = parent::request($uri, $options);
    }
    catch(Exception $e) {
      watchdog('mediamosa_connector_ex exception', sprintf('%s (Line: %s, File: %s, severity level: %d)', $e->getMessage(), $e->getLine(), $e->getFile(), WATCHDOG_ERROR));
      // Exception lead to FALSE as a result.
      return FALSE;
    }
    if (!$this->check_result($response)) {
      watchdog('mediamosa_connector_ex error', $response->xml->header->request_result_description, WATCHDOG_ERROR);
      return FALSE;
    }
    return $response;
  }

  /**
   * @param $xml_response
   *
   */
  public function get_items_array($xml_response) {
    $items = array();
    if (isset($xml_response->items) && isset($xml_response->items->item)) {
      foreach ($xml_response->items->item as $obj_item) {
        $items[] = (array)$obj_item;
      }
    }
    return $items;
  }
}
