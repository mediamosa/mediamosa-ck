<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
 * Still file.
 */

class MediaMediaMosaFileStyles extends FileStyles {
  public $autoplay;
  public $fullscreen;

  function get_autoplay() {
    return $this->get('autoplay');
  }

  function set_autoplay($value) {
    return $this->set('autoplay', $value);
  }

  function get_fullscreen() {
    return $this->get('fullscreen');
  }

  function set_fullscreen($value) {
    return $this->set('fullscreen', $value);
  }

  function getImageUri() {
    $image_uri = $this->get('image_uri');
    if ($image_uri) {
      return $image_uri;
    }
    $object = $this->getObject();
    if ($object->uri) {
      // Get the asset_id and the local path, where the still image may exists.
      $params = MediaMosaStreamWrapper::mediamosa_parse_url($object->uri);

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
          return NULL; // No image link.
        }
      }

      return $local_path;
    }
  }

  function video($effect) {
    $params = MediaMosaStreamWrapper::mediamosa_parse_url($this->getUri());
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
      //'fullscreen' => $this->get_fullscreen(),
      'response' => 'object',
    );
    $output = MediaMosaCkConnectorWrapper::get_play_link($asset_id, $mediafile_id_transcoded, $user_id, $options);

    $this->setOutput($output);
  }
}
