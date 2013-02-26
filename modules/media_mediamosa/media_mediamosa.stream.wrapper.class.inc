<?php
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
 */
class MediaMosaStreamWrapper extends MediaReadOnlyStreamWrapper {

  /**
   * No getTarget().
   */
  function getTarget($f) {
    return FALSE;
  }

  /**
   * InterpolateUrl().
   */
  function interpolateUrl() {
    $parameters = $this->get_parameters();
    if (empty($parameters)) {
      return FALSE;
    }

    // @todo: this needs a user id parameter.
    return MediaMosaCkConnectorWrapper::get_play_link($parameters['asset_id'], $parameters['mediafile_id']);
  }

  /**
   * Returns an array of any parameters stored in the URL's path.
   *
   * @param string $url
   *   The URL to parse, such as
   *   mediamosa://asset/[asset_id]/[mediafile_id]/file_name.
   *
   * @return array
   *   An associative array of all the parameters in the path,
   *   or FALSE if the $url is ill-formed.
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
   *   The url to parse
   *
   * @return array
   *   Either array filled or FALSE.
   */
  public static function mediamosa_parse_url($url) {
    // Parse the url.
    $matches = array();
    if (!preg_match('@^mediamosa://asset/([a-zA-Z0-9]+)/([a-zA-Z0-9]+)/([^/]+)$@i', $url, $matches)) {
      return FALSE;
    }

    // Get the values.
    $params = array(
      'asset_id' => $matches[1],
      'mediafile_id' => $matches[2],
      'filename' => $matches[3],
    );

    return $params;
  }

  /**
   * Get and split up the URL into its parts.
   *
   * @param integer $fid
   *   The Drupal file ID.
   *
   * @return array
   *   Either array filled or FALSE.
   */
  public static function mediamosa_parse_url_from_fid($fid) {
    $media = reset(media_multi_load($fid));
    if (!$media || !isset($media->uri)) {
      return FALSE;
    }

    return self::mediamosa_parse_url($media->uri);
  }

  /**
   * Translate non standard video mimetypes.
   *
   * Some mimetypes are valid, but might not match some of the Media definition
   * of mimetypes. So are some mimetypes not seen as video types, because they
   * don't start with 'video/'.
   *
   * @param string $mimetype
   *   Get the mimetype.
   */
  public static function translateMimeType($mimetype) {
    static $translate = array(
      'application/ogg' => 'video/ogg',
    );

    // Translate.
    return (isset($translate[$mimetype]) ? $translate[$mimetype] : $mimetype);
  }

  /**
   * The mimetype returned is based on the URI and content we return.
   *
   * @param string $url
   *   url
   */
  public static function getMimeType($url, $mapping = NULL) {
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

        // Return the mime-type, if any.
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
   * @param string $asset_id
   *   The asset_id for the still.
   *
   * @return string
   *   The url to the still.
   *
   * @see getOriginalThumbnailPath()
   */
  public static function mediamosa_getOriginalThumbnailPath($asset_id) {

    try {
      // Get the still url.
      $asset = MediaMosaCkConnectorWrapper::get_asset($asset_id);
      $thumb = (string) $asset->items->item->vpx_still_url;
    }
    catch (Exception $e) {
    }

    // What if we have no thumb.
    if (empty($thumb)) {
      // If no thumbnail, but the original is audio, we present an audio image.
      $mime_type = @$asset->items->item->mediafiles->mediafile->metadata->mime_type;
      if (substr($mime_type, 0, 5) == 'audio') {
        $thumb = DRUPAL_ROOT . '/' . drupal_get_path('module', 'mediamosa_ck') . '/modules/mediamosa_ck_views/images/' . 'audio.jpg';
      }
      else {
        // If all failes we present an borken image.
        $thumb = DRUPAL_ROOT . '/' . drupal_get_path('module', 'mediamosa_ck') . '/modules/mediamosa_ck_views/images/' . 'notfound.jpg';
      }
    }
    return $thumb;
  }

  /**
   * Returns the still url of the asset.
   *
   * @return string
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
   * @param string $asset_id
   *   The asset_id for the still.
   *
   * @return string
   *   The local path of the still.
   *
   * @see getLocalThumbnailPath()
   */
  public static function mediamosa_getLocalThumbnailPath($asset_id) {
    $local_path = 'public://media-mediamosa/' . $asset_id . '.jpg';
    if (!file_exists($local_path) || time() - filectime($local_path) > MediaMediaMosa::THUMBNAIL_EXPIRE_TIME) {
      $original_thumb = self::mediamosa_getOriginalThumbnailPath($asset_id);
      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      copy($original_thumb, $local_path);
    }
    return $local_path;
  }

  /**
   * Returns the local version path of the still.
   *
   * @return string
   *   The local path of the still.
   *
   * @see mediamosa_getLocalThumbnailPath()
   */
  public function getLocalThumbnailPath() {
    $parts = $this->get_parameters();
    return self::mediamosa_getLocalThumbnailPath($parts['asset_id']);
  }
}
