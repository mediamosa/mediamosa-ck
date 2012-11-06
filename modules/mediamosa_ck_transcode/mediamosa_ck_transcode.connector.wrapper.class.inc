<?php
/**
 * @file
 * Wrapper around the mediamosa_connector.
 */

class MediaMosaCkConnectorTranscodeWrapper extends MediaMosaCkConnectorWrapper {
  /**
   * Retrieving profile names.
   *
   * @deprecated: uses deprecated function response2array().
   *
   * @return array
   *   array of transcode profile names.
   */
  public static function get_profile_names() {

    try {
      // Do request.
      $response = mediamosa_ck::request_get_fatal('transcode/profile');
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to update metadata; @message.', array('@message' => $e->getMessage()));
      return FALSE;
    }

    // Convert to array.
    $xml = mediamosa_connector::response2array($response);

    // Return our items.
    return !empty($xml['items']['item']) ? $xml['items']['item'] : array();
  }

  /**
   * Retrieving all non-finished jobs.
   *
   * @param string $asset_id
   *   ID of asset.
   * @param string $owner_id
   *   If you already know an owner please supply; this will save from extra
   *   request.
   *
   * @return array
   *   Array with jobs.
   */
  public static function get_asset_actual_jobs($asset_id, $owner_id = FALSE) {

    $uri = strtr('asset/@asset_id/joblist', array('@asset_id' => $asset_id));

    if (!$owner_id) {
      $owner_id = self::get_asset_owner($asset_id);
    }

    $data = array(
      'cql' => 'NOT status == "^FINISHED^"',
      'user_id' => $owner_id,
    );

    try {
      // Do request.
      $response = mediamosa_ck::request_get_fatal($uri, array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to get job list; @message.', array('@message' => $e->getMessage()));
      return FALSE;
    }

    $xml = mediamosa_connector::response2array($response);

    $jobs = !empty($xml['items']['item']) ? $xml['items']['item'] : array();

    return $jobs;
  }

  /**
   * Delete a particular job.
   *
   * @param string $job_id
   *   id of job to delete.
   * @param string $owner_id
   *   owner id of job.
   *
   * @return boolean
   *   returns TRUE if succes.
   */
  public static function delete_job($job_id, $owner_id) {

    // REST uri.
    $uri = strtr('job/@job_id/delete', array('@job_id' => $job_id));

    try {
      $result = mediamosa_ck::request_post_fatal($uri, array('user_id' => $owner_id));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete job; @message.', array('@message' => $e->getMessage()));
      return FALSE;
    }
    return $result->code == 200;
  }
}
