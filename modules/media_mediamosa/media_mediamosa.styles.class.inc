<?php
/**
 * @file
 * Still file.
 */

class MediaMediaMosaFileStyles extends FileStyles {
  public $autoplay;
  public $fullscreen;

  /**
   * Autoplay getter.
   */
  public function get_autoplay() {
    return $this->get('autoplay');
  }

  /**
   * Autoplay setter.
   */
  public function set_autoplay($value) {
    return $this->set('autoplay', $value);
  }

  /**
   * Fullscreen getter.
   */
  public function get_fullscreen() {
    return $this->get('fullscreen');
  }

  /**
   * Fullscreen setter.
   */
  public function set_fullscreen($value) {
    return $this->set('fullscreen', $value);
  }

  /**
   * Get URI of image.
   */
  public function getImageUri() {
    $image_uri = $this->get('image_uri');
    if ($image_uri) {
      return $image_uri;
    }
    $object = $this->getObject();
    if ($object->uri) {
      // Get the asset_id and the local path, where the still image may exists.
      $params = mediamosa_ck::fileuri2parts($object->uri);

      if (!$params) {
        return FALSE;
      }

      $asset_id = $params['asset_id'];
      $local_path = 'public://media-mediamosa/' . $asset_id . '.jpg';

      // Copy the still url, if it necessary.
      if (!file_exists($local_path) || time() - filectime($local_path) > MediaMediaMosa::THUMBNAIL_EXPIRE_TIME) {

        // Unlink first.
        @unlink($local_path);
        try {
          // Get the still url.
          $asset = MediaMosaCkConnectorWrapper::get_asset($asset_id);
          $still_url = (string) $asset->items->item->vpx_still_url;

          // Test before usage.
          if (empty($still_url)) {
            return FALSE;
          }

          // Copy it to local.
          $dirname = drupal_dirname($local_path);
          file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
          @copy($still_url, $local_path);
        }
        catch (Exception $e) {
          // No image link.
          return NULL;
        }
      }

      return $local_path;
    }
  }

  /**
   * Get Media.
   */
  public function video($effect) {
    $params = mediamosa_ck::fileuri2parts($this->getUri());
    $asset_id = $params['asset_id'];
    $mediafile_id = $params['mediafile_id'];
    $mediafile_id_transcoded = $mediafile_id;

    $asset = MediaMosaCkConnectorWrapper::get_asset($asset_id);
    $user_id = (string) $asset->items->item->owner_id;
    if (isset($asset->items->item->mediafiles->mediafile)) {
      foreach ($asset->items->item->mediafiles->mediafile as $mediafile) {
        if ((string) $mediafile->mediafile_id_source == $mediafile_id) {
          $mediafile_id_transcoded = (string) $mediafile->mediafile_id;
          break;
        }
      }
    }

    $options = array(
      'autostart' => $this->get_autoplay(),
      'height' => $this->getHeight(),
      'width' => $this->getWidth(),
      // 'fullscreen' => $this->get_fullscreen(),
      'response' => 'object',
    );
    $output = MediaMosaCkConnectorWrapper::get_play_link($asset_id, $mediafile_id_transcoded, $user_id, $options);

    $this->setOutput($output);
  }
}
