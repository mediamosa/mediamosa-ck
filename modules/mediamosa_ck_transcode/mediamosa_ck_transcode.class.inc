<?php
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

class mediamosa_ck_transcode {

  // The default transcode profile.
  const MEDIA_MEDIAMOSA_TRANSCODE_PROFILE_DEFAULT = 2;

  /**
   * Return the default transcode profile.
   *
   * @return
   *   The transcode profile ID.
   */
  static public function get_default_transcode_profile() {

    $transcode_profile_default = mediamosa_connector::variable_get('media_mediamosa_transcode_profile_default', 0);
    if (!$transcode_profile_default) {

      // Get the profiles.
      $profiles = MediaMosaCkConnectorTranscodeWrapper::get_transcode_profiles();
      if (!$profiles) {
        mediamosa_ck::watchdog_error(t('Unable to retrieve transcode profiles.'));
      }

      $transcode_profile_default = self::MEDIA_MEDIAMOSA_TRANSCODE_PROFILE_DEFAULT;
      if ($profiles && isset($profiles->items->item)) {
        foreach ($profiles->items->item as $profile_id => $profile) {

          // Set to default?
          if (drupal_strtoupper((string) $profile->default) == 'TRUE') {
            $transcode_profile_default = (int) $profile->profile_id;
            // FIFO.
            break;
          }
        }
      }
    }

    return $transcode_profile_default;
  }

  /**
   * Return the allowed transcode profiles.
   *
   * @return
   *   The transcode profile ID.
   */
  static public function get_allowed_transcode_profiles() {

    // Get the allowed transcode profile.
    $transcode_profile_allowed = mediamosa_connector::variable_get('media_mediamosa_transcode_profile_allowed', array());

    // Not allowed set? Then get them all and allow them all.
    if (empty($transcode_profile_allowed)) {
      // Get the profiles.
      $profiles = MediaMosaCkConnectorTranscodeWrapper::get_transcode_profiles();
      if (!$profiles) {
        mediamosa_ck::watchdog_error(t('Unable to retrieve transcode profiles.'));
      }

      if ($profiles && isset($profiles->items->item)) {
        foreach ($profiles->items->item as $profile_id => $profile) {
          $transcode_profile_allowed[(int) $profile->profile_id] = (string) $profile->profile;
        }
      }

      // Default is all, so safe it so next time we don't need to do the REST
      // query again.
      if (!empty($transcode_profile_allowed)) {
        $transcode_profile_allowed = array_keys($transcode_profile_allowed);
        mediamosa_connector::variable_set('media_mediamosa_transcode_profile_allowed', $transcode_profile_allowed);
      }
    }

    return $transcode_profile_allowed;
  }
}