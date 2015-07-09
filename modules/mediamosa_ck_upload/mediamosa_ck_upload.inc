<?php
/**
 * @file
 * All non-hooks are put here.
 */

/**
 * Default upload URL that prevents upload to Drupal.
 *
 * This function is attached to mediamosa/ck/upload/die. It can only be called
 * when upload has not been directed to MediaMosa, but to this front end. Will
 * do die() to stop any process of the uploaded file.
 */
function _mediamosa_ck_upload_die() {
  die('{"jsonrpc" : "2.0", "error" : {"code": 104, "message": "Access to local upload denied."}, "id" : "id"}');
}

/**
 * Process the file upload.
 *
 * @param string $mediafile_id
 *   The mediafile_id of the file that has been uploaded.
 */
function _mediamosa_ck_json_file_uploaded($mediafile_id) {

  $file_uploaded = array('status' => 'ok');
  if (mediamosa_ck_upload::get_allow_bagit_upload()) {
    try {
      $mediafile_xml = MediaMosaCkConnectorWrapper::get_mediafile($mediafile_id);
      $filename = (string) $mediafile_xml->items->item->filename;
      $start_transcode_bagit = false;
      foreach (explode(' ', mediamosa_ck_upload::get_bagit_file_ext()) as $ext) {
        $ext = trim($ext, ". \t\n\r\0\x0B");
        $ext = '.' . $ext;
        if (drupal_substr($filename, -drupal_strlen($ext)) == $ext) {
          $start_transcode_bagit = true;
          break;
        }
      }

      if ($start_transcode_bagit) {
        $profile_id = mediamosa_ck_upload::get_bagit_transcode_profile();
        if ($profile_id > 0) {
          MediaMosaCkConnectorWrapper::create_transcode($mediafile_id, array('profile_id' => $profile_id));
        }
      }
    }
    catch (Exception $e) {
      $file_uploaded = array('status' => 'failed', 'message' => $e->getMessage());
    }
  }

  module_invoke_all("mediamosa_ck_asset_uploaded");

  // Output as JSON.
  drupal_json_output($file_uploaded);
}