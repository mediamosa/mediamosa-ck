<?php
/**
 * @file
 * Pubish / unpublish assets plugin.
 */

class mediamosa_ck_media_management_queue_plugin_publish extends mediamosa_ck_media_management_queue_plugin {

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
    return self::publish_form($form, $form_state);
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
    $options = array(
      'isprivate' => $data['publish'] == 1 ? 'FALSE' : 'TRUE',
      'user_id' => $user_id,
    );

    MediaMosaCkConnectorWrapper::update_asset($response->get_asset_id(), $options);

    return $data;
  }

  /**
   * Publish options.
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   *
   * @return array
   *   The transcode settings form.
   */
  public static function publish_form($form, &$form_state) {
    $form['publish'] = array(
      '#type' => 'radios',
      '#title' => t('Do you want to publish or unpublish the selected assets?'),
      '#options' => array(1 => t('Publish'), 0 => t('Unpublish')),
      '#required' => TRUE,
    );

    return $form;
  }
}
