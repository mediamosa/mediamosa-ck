<?php
/**
 * @file
 * The basic CK upload class.
 */

class mediamosa_ck_upload {
  // ------------------------------------------------------------------- Consts.
  // Default max file size, about 2gb.
  const MAX_FILE_SIZE = 2000;

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
   * Helper function to add defaults to $element['#upload_validators'].
   */
  public static function get_default_upload_validators() {
    return array('avi mov mp4 ogg wmv');
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
