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
 * Class stream wrapper for MediaMosa.
 */

/**
 * Create an instance url for streaming directly using url.
 *
 * $mediamosa = new MediaMosaStreamWrapper('mediamosa://');
 *
 * Formats;
 * mediamosa://asset/[asset_id]/[mediafile_id]/[filename]
 *
 *
 *
 * @see class MediaFlickrStreamWrapper and MediaYouTubeStreamWrapper for
 *      (bad) examples.
 */
class MediaMosaStreamWrapper extends MediaReadOnlyStreamWrapper {

  function getTarget($f) {
    return FALSE;
  }

  function interpolateUrl() {
    $parameters = $this->get_parameters();
    if (empty($parameters)) {
      return FALSE;
    }

    return MediaMosaCkConnectorWrapper::request_get_play_link($parameters['asset_id'], $parameters['mediafile_id']);
  }

  /**
   *  Returns an array of any parameters stored in the URL's path.
   *  @param $url
   *    The URL to parse, such as mediamosa://asset/[asset_id]/[mediafile_id]/file_name
   *  @return
   *    An associative array of all the parameters in the path,
   *    or FALSE if the $url is ill-formed.
   */
  protected function _parse_url($url) {

    // Get the params.
    $params = self::mediamosa_parse_url($url);

    // Any params? Else just return parent.
    return empty($params) ? parent::_parse_url($url) : $params;
  }

  /**
   * Split up the URL into its parts.
   *
   * @param string $url
   *  The url to parse
   *
   * @return array
   *  Either array filled or FALSE.
   */
  static public function mediamosa_parse_url($url) {
        // Parse the url.
    if (!preg_match('@^mediamosa://asset/([a-zA-Z0-9]+)/([a-zA-Z0-9]+)/([^/]+)$@i', $url, $matches)) {
      return FALSE;
    }

    // Get the values.
    $params['asset_id'] = $matches[1];
    $params['mediafile_id'] = $matches[2];
    $params['filename'] = $matches[3];

    return $params;
  }

  /**
   * Get and split up the URL into its parts.
   *
   * @param ind $fid
   *
   * @return array
   *  Either array filled or FALSE.
   */
  static public function mediamosa_parse_url_from_fid($fid) {
    $media_query = media_multi_load("{$fid}");
    $media = $media_query[$fid];

    if (!$media || !isset($media->uri)) {
      return FALSE;
    }

    return self::mediamosa_parse_url($media->uri);
  }

  /**
   * The mimetype returned is based on the URI and content we return.
   *
   * @param string $uri
   * @param string $mapping
   */
  static public function getMimeType($url, $mapping = NULL) {

    $mimetypes = &drupal_static(__CLASS__ . '::' . __FUNCTION__);

    // If it fails, then its not mediamosa.
    $params = self::mediamosa_parse_url($url);

    if (!empty($params)) {
      // Now get the Mediafile (is cached when already used).
      try {
        // Cached?
        if (!empty($mimetypes[$params['mediafile_id']])) {
          return $mimetypes[$params['mediafile_id']];
        }

        $mediafile_xml = MediaMosaCkConnectorWrapper::request_get_mediafile($params['mediafile_id']);
        $mimetypes[$params['mediafile_id']] = empty($mediafile_xml->items->item->metadata->mime_type) ? 'application/octet-stream' : (string) $mediafile_xml->items->item->metadata->mime_type;

        // return the mime-type, if any.
        return $mimetypes[$params['mediafile_id']];
      }
      catch (Exception $e) {
        // Failed. Return default.
        return 'application/octet-stream';
      }
    }

    return 'application/octet-stream';
  }
}
