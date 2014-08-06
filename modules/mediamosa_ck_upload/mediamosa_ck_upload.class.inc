<?php

/**
 * @file
 * The basic CK upload class.
 */

class mediamosa_ck_upload {

  // ------------------------------------------------------------------- Consts.
  // Default max file size, about 2gb.
  const MAX_FILE_SIZE = 2000;
  const ACCESS_TOOLS_UPLOAD = 'access to bulk upload tool';

  // ---------------------------------------------------------------- Functions.
  /**
   * Returns the path to the plupload library.
   */
  public static function get_library_path() {
    return variable_get('mediamosa_ck_upload_library_path', module_exists('libraries') ? libraries_get_path('plupload') : 'sites/all/libraries/plupload');
  }

  /**
   * Get the maximum client file size.
   *
   * @return integer
   *   The number of megabytes.
   */
  public static function get_max_file_size() {
    return variable_get('mediamosa_ck_upload_max_file_size', mediamosa_ck_upload::MAX_FILE_SIZE);
  }

  /**
   * Return the setting for allowed CK Upload allowed file extension.
   *
   * @return string
   *   The allowed file ext string setting.
   */
  public static function get_allowed_file_ext($as_array = FALSE) {
    $mediamosa_ck_upload_allowed_file_ext = variable_get('mediamosa_ck_upload_allowed_file_ext', 'avi mov mp4 mpg m4v ogv wmv 3gp vob divx dv mpeg flv webm srt mp3 m4a ogg wav bmp pdf jpg jpeg gif tiff tif png');
    return $as_array ? explode(' ', $mediamosa_ck_upload_allowed_file_ext) : $mediamosa_ck_upload_allowed_file_ext;
  }

  /**
   * Return the setting for allowing bagit upload.
   *
   * @return integer
   *   The allowed file ext string setting.
   */
  public static function get_allow_bagit_upload() {
    return variable_get('mediamosa_ck_upload_allow_bagit_upload', FALSE);
  }

  /**
   * Return the setting for allowed CK Upload allowed file extension.
   *
   * @return string
   *   The allowed file ext string setting.
   */
  public static function get_bagit_file_ext($as_array = FALSE) {
    $mediamosa_ck_upload_bagit_file_ext = variable_get('mediamosa_ck_upload_bagit_file_ext', 'zip tar.gz tgz');
    return $as_array ? explode(' ', $mediamosa_ck_upload_bagit_file_ext) : $mediamosa_ck_upload_bagit_file_ext;
  }

  /**
   * Return the setting for allowed CK Upload allowed file extension.
   *
   * @return integer
   *   The bagit transcode profile ID.
   */
  public static function get_bagit_transcode_profile() {
    return variable_get('mediamosa_ck_upload_bagit_profile', 0);
  }

  /**
   * Return the setting to specify if bulk uploads are private.
   *
   * @return integer
   *   Private or not
   */
  public static function get_isprivate() {
    return variable_get('mediamosa_ck_mediafile_isprivate', 0);
  }

  /**
   * Helper function to add defaults to $element['#upload_validators'].
   */
  public static function get_default_upload_validators() {
    $mediamosa_ck_upload_allowed_file_ext = self::get_allowed_file_ext();

    // Include settings for bagit upload.
    if (self::get_allow_bagit_upload()) {
      $mediamosa_ck_upload_allowed_file_ext .= ' ' . self::get_bagit_file_ext();
    }

    $default_ext = explode(' ', $mediamosa_ck_upload_allowed_file_ext);
    if (function_exists('media_variable_get')) {
      $default_ext += explode(' ', media_variable_get('file_extensions'));
    }
    return array(implode(' ', $default_ext));
  }

  /**
   * Checks wether Plupload library exists or not.
   *
   * @return boolean
   *   TRUE if plupload library installed, FALSE otherwise.
   */
  public static function requirements_installed() {
    $libraries = mediamosa_ck_upload_library();

    // We grab the first file and check if it exists.
    return file_exists(key($libraries['plupload']['js']));
  }

  /**
   * Returns the version of the installed plupload library.
   *
   * @return string
   *   The version of installed plupload or NULL if unable to detect version.
   */
  public static function requirements_version() {
    $library_path = self::get_library_path();
    $jspath = $library_path . '/js/plupload.js';

    // Read contents of Plupload's javascript file.
    $configcontents = @file_get_contents($jspath);
    if ($configcontents) {
      // Search for version string using a regular expression.
      $matches = array();
      if (preg_match('#VERSION:\"(\d+[\.\d+]*)\"#', $configcontents, $matches)) {
        return $matches[1];
      }
    }

    return NULL;
  }
}
