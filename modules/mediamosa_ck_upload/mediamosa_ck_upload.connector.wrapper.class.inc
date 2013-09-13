<?php
/**
 * @file
 * Wrapper around the mediamosa_connector.
 */

class MediaMosaCkConnectorUploadWrapper extends MediaMosaCkConnectorWrapper {
  /**
   * Return the transcode profiles that are used after upload.
   *
   * @param array $options
   *   - refresh_title (default TRUE)
   *     Reload the titles from MediaMosa.
   *
   * @return integer
   *   The transcode profile ID.
   */
  public static function get_upload_transcode_profiles(array $options = array()) {
    // Default options.
    $options += array(
      'refresh_title' => TRUE,
    );

    // Get the allowed transcode profile.
    $transcode_profiles_allowed = mediamosa_connector::variable_get('mediamosa_ck_upload_transcode_profiles', array());
    if (!empty($transcode_profiles_allowed) && !$options['refresh_title']) {
      return $transcode_profiles_allowed;
    }

    // Get the profiles.
    $profiles = MediaMosaCkConnectorWrapper::get_transcode_profiles();
    if (!$profiles) {
      mediamosa_ck::watchdog_error(t('Unable to retrieve transcode profiles.'));
    }

    // No allowed set? Then get them all and allow them all.
    if (empty($transcode_profiles_allowed)) {
      // Get the profiles.
      foreach ($profiles->items->item as $profile_id => $profile) {
        $transcode_profiles_allowed[(int) $profile->profile_id] = t((string) $profile->profile);
      }
    }
    else {
      // Mark them so we remove 'removed' transcode profiles.
      foreach (array_keys($transcode_profiles_allowed) as $key) {
        $transcode_profiles_allowed[$key] = FALSE;
      }

      // Get the profiles.
      foreach ($profiles->items->item as $profile_id => $profile) {
        // Reset the title.
        if (isset($transcode_profiles_allowed[(int) $profile->profile_id])) {
          $transcode_profiles_allowed[(int) $profile->profile_id] = t((string) $profile->profile);
        }
      }

      // Any with FALSE value is no longer present in mediamosa.
      foreach (array_keys($transcode_profiles_allowed) as $key) {
        if ($transcode_profiles_allowed[$key] === FALSE) {
          unset($transcode_profiles_allowed[$key]);
        }
      }
    }

    // Store them for later retrieval.
    if (!empty($transcode_profiles_allowed)) {
      mediamosa_connector::variable_set('mediamosa_ck_upload_transcode_profiles', $transcode_profiles_allowed);
    }

    return $transcode_profiles_allowed;
  }


}
