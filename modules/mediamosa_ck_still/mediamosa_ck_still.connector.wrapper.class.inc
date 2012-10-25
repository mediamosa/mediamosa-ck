<?php
/**
 * @file
 * Our wrapper around the MediaMosaCkConnectorWrapper.
 */

class MediaMosaCkConnectorStillWrapper extends MediaMosaCkConnectorWrapper {

  /**
   * Create a still for specific mediafile. Code moved to
   * MediaMosaCkConnectorWrapper.
   *
   * @param $mediafile_id
   *   The mediafile ID.
   * @param $user_id
   *   The owner of the mediafile.
   * @param $data
   *   Array with all the still parameters.
   *
   * @return mediamosa_connector_response
   *   Returns FALSE when failure or mediamosa_connector_response.
   */
  public static function still_create($mediafile_id, $user_id, array $data = array()) {
    return parent::create_mediafile_still($mediafile_id, $user_id, $data);
  }

  /**
   * Set still default.
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $still_id
   * @param string $user_id
   * @param array $data
   */
  public static function still_default($asset_id, $mediafile_id, $still_id, $user_id, array $data = array()) {
    // Check the parameters.
    if (empty($asset_id) || empty($mediafile_id) || empty($still_id) || empty($user_id)) {
      return FALSE;
    }

    $data += array(
      'user_id' => $user_id,
      'mediafile_id' => $mediafile_id,
      'still_id' => $still_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/still/default', array('data' => $data));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to start a still process, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }

  /**
   * Delete still.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $still_id
   *   The still ID to delete.
   * @param string $user_id
   *   The owner of the asset.
   * @param array $data
   */
  public static function still_delete($asset_id, $mediafile_id, $still_id, $user_id, $options = array()) {
    // Check the parameters.
    if (empty($asset_id) || empty($mediafile_id) || empty($still_id) || empty($user_id)) {
      return FALSE;
    }

    $options += array(
      'user_id' => $user_id,
      'mediafile_id' => $mediafile_id,
      'still_id' => $still_id,
    );

    try {
      // Do request.
      return mediamosa_ck::request_post_fatal('asset/' . rawurlencode($asset_id) . '/still/delete', array('data' => $options));
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to delete still, reason; @message.', array('@message' => $e->getMessage()));
    }

    return FALSE;
  }
}
