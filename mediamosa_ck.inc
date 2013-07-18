<?php
/**
 * @file
 * All non-hooks are put in this file.
 */

/**
 * Create the upload ticket and return result array in json.
 *
 * @return array
 *   Returns an associative array;
 *   - 'action'
 *     The upload URL for the file.
 *   - 'progress_id'
 *     Progress ID to use when getting upload progression from server.
 *   - 'asset_id'
 *     The asset ID of the upload.
 *   - 'mediafile_id'
 *     The mediafile ID of the upload.
 */
function _mediamosa_ck_json_create_upload_ticket() {

  // Create upload ticket.
  $upload_ticket = MediaMosaCkConnectorWrapper::create_upload_ticket();

  // Output as JSON.
  drupal_json_output($upload_ticket);
}

/**
 * Check the connector status, if connector works and is setup.
 *
 * @return string
 *   JSON containing '1' for OK, '0' for failure.
 */
function _mediamosa_ck_json_connector_status() {
  // Get the connector.
  $connector = mediamosa_ck::get_connector();

  $result = array(
    'ok' => $connector->check_connection() ? '1' : '0',
  );

  // Return '1' for success, '0' for failure.
  drupal_json_output($result);
}

/**
 * Download mediafile.
 */
function _mediamosa_ck_mediafile_download($asset_id, $mediafile_id) {

  // Take user_id of current watcher.
  $user_id = mediamosa_ck::session_user_id();

  // Determine admin user.
  if (user_access('access administration pages')) {
    $options['is_app_admin'] = TRUE;
  }

  try {
    $options['fatal'] = TRUE;
    $options['response'] = 'download';
    $response = MediaMosaCkConnectorWrapper::get_play_link($asset_id, $mediafile_id, $user_id, $options);
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'warning');
    drupal_goto(strtr('asset/detail/@asset_id/@mediafile_id', array('@asset_id' => $asset_id, '@mediafile_id' => $mediafile_id)));
  }
  drupal_goto($response);
}
