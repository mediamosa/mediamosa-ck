<?php
/**
 * @file
 * Internet media handler class.
 */

class MediaInternetMediaMosaHandler extends MediaInternetBaseHandler {

  public function claim($embed_code) {
    if (media_mediamosa_media_parse($embed_code)) {
      return TRUE;
    }
  }

  public function validate() {
    // @todo Media module currently fails when two files try to have the same
    //   URI, so catch that in the validation step. Some day, it would be nice
    //   to allow it, however. See http://drupal.org/node/952422.
    $uri = media_mediamosa_media_parse($this->embedCode);
    $existing_files = file_load_multiple(array(), array('uri' => $uri));
    if (count($existing_files)) {
      throw new MediaInternetValidationException(t('You have entered a URL for a video that is already in your library.'));
    }
  }

  public function save() {
    $file = $this->getFileObject();
    file_save($file);
    return $file;
  }

  public function getFileObject() {
    $uri = media_mediamosa_media_parse($this->embedCode);
    return file_uri_to_object($uri);
  }
}
