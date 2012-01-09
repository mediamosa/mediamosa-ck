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

    return MediaMosaCkConnectorWrapper::get_play_link($parameters['asset_id'], $parameters['mediafile_id']);
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
   * @param $fid
   *   The Drupal file ID.
   *
   * @return array
   *   Either array filled or FALSE.
   */
  static public function mediamosa_parse_url_from_fid($fid) {
    $media = reset(media_multi_load($fid));
    if (!$media || !isset($media->uri)) {
      return FALSE;
    }

    return self::mediamosa_parse_url($media->uri);
  }

  /**
   * Some mimetypes are valid, but might not match some of the Media definition
   * of mimetypes. So are some mimetypes not seen as video types, because they
   * don't start with 'video/'.
   *
   * @param $mimetype
   *   Get the mimetype.
   */
  static public function translateMimeType($mimetype) {
    static $translate = array(
      'application/ogg' => 'video/ogg',
    );

    // Translate.
    return (isset($translate[$mimetype]) ? $translate[$mimetype] : $mimetype);
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

        $mediafile_xml = MediaMosaCkConnectorWrapper::get_mediafile($params['mediafile_id']);
        $mimetypes[$params['mediafile_id']] = self::translateMimeType(empty($mediafile_xml->items->item->metadata->mime_type) ? 'application/octet-stream' : (string) $mediafile_xml->items->item->metadata->mime_type);

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

  /**
   * Returns the still url of the asset. Made static for external access.
   *
   * @param $asset_id
   *   The asset_id for the still.
   *
   * @return
   *   The url to the still.
   *
   * @see getOriginalThumbnailPath()
   */
  static public function mediamosa_getOriginalThumbnailPath($asset_id) {
    try {
      // Get the still url.
      $asset = MediaMosaCkConnectorWrapper::get_asset($asset_id);
      $thumb = (string) $asset->items->item->vpx_still_url;
    }
    catch (Exception $e) {
    }

    return empty($thumb) ? FALSE : $thumb; // Return FALSE when no still.
  }

  /**
   * Returns the still url of the asset.
   *
   * @return
   *   The url to the still.
   *
   * @see mediamosa_getOriginalThumbnailPath()
   */
  public function getOriginalThumbnailPath() {
    $parts = $this->get_parameters();
    return self::mediamosa_getOriginalThumbnailPath($parts['asset_id']);
  }

  /**
   * Returns the local version path of the still.
   *
   * @param $asset_id
   *   The asset_id for the still.
   *
   * @return
   *   The local path of the still.
   *
   * @see getLocalThumbnailPath()
   */
  static public function mediamosa_getLocalThumbnailPath($asset_id) {
    $local_path = 'public://media-mediamosa/' . $asset_id . '.jpg';
    if (!file_exists($local_path) || time() - filectime($local_path) > MediaMediaMosa::THUMBNAIL_EXPIRE_TIME) {
      $original_thumb = self::mediamosa_getOriginalThumbnailPath($asset_id);
      if (empty($original_thumb)) {
        return NULL;
      }

      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      @copy($original_thumb, $local_path);
    }
    return $local_path;
  }

  /**
   * Returns the local version path of the still.
   *
   * @return
   *   The local path of the still.
   *
   * @see mediamosa_getLocalThumbnailPath()
   */
  public function getLocalThumbnailPath() {
    $parts = $this->get_parameters();
    return self::mediamosa_getLocalThumbnailPath($parts['asset_id']);
  }
}
