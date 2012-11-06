<?php
/**
 * @file
 * Class stream wrapper for MediaMosa.
 *
 * @todo: needs work.
 */

/**
 * Wrapper for getting the thumbnails of the media.
 */
class MediaMosaThumbWrapper extends DrupalLocalStreamWrapper {

  /**
   * Base implementation of getMimeType().
   */
  public static function getMimeType($uri, $mapping = NULL) {
    return 'image/jpeg';
  }


}
