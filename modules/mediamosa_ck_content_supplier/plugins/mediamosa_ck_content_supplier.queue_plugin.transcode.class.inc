<?php
/**
 * @file
 * Start transcodes plugin.
 */

class mediamosa_ck_content_supplier_queue_plugin_transcode extends mediamosa_ck_content_supplier_queue_plugin {

  /**
   * Get the settings form.
   *
   * Return empty array or FALSE when the process has no settings.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   * @param array $batch
   *   The batch data.
   *
   * @return array
   *   The settings form.
   */
  public static function settings_form($form, &$form_state, $batch) {
    return self::transcode_form($form, $form_state);
  }

  /**
   * Process the asset.
   *
   * @param mediamosa_connector_response_asset $response
   *   The response from the asset call.
   * @param array $data
   *   Array of data that has been stored from the last process_asset() call.
   *   The 'form_values' contains the submit of the possible options form
   *   submit.
   *
   * @return array
   *   Return the data array again to be stored for the next call.
   */
  public function process_asset(mediamosa_connector_response_asset $response, array $data = array()) {
    $user_id = $this->get_session_id();

    // Find original mediafiles, loop them.
    $mediafiles = $response->get_original_mediafiles();
    foreach ($mediafiles as $mediafile_id => $mediafile) {
      // And start the choosen transcodes.
      foreach ($data['media_mediamosa_transcodes'] as $transcode_profile) {
        if ($transcode_profile > 0) {
          MediaMosaCkConnectorWrapper::create_transcode($mediafile_id, array('profile_id' => $transcode_profile, 'user_id' => $user_id));
        }
      }
    }

    return $data;
  }

  /**
   * Transcode parameters.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   *
   * @return array
   *   The transcode settings form.
   */
  public static function transcode_form($form, &$form_state) {
    // This batch will start the choosen transcode job(s) on the original
    // mediafiles of the assets in the batch.
    $available_transcode_profiles = variable_get('mediamosa_ck_transcode_profiles_allowed');

    $form['media_mediamosa_upload']['media_mediamosa_transcodes'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Start transcodes:'),
      '#options' => $available_transcode_profiles,
      '#required' => TRUE,
    );

    return $form;
  }
}
