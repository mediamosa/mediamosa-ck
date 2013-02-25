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
