<?php
/**
 * @file
 * Delete assets in queue plugin.
 */

class mediamosa_ck_content_supplier_queue_plugin_delete_asset extends mediamosa_ck_content_supplier_queue_plugin {

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
    $item_count = $batch['total'];

    // Setup description.
    $question = format_plural(
      $item_count,
      'There is one asset in the batch to delete. Are you sure?',
      'There are @count assets in the batch for deletion. Are you sure?',
      array('@count' => $item_count)
    );

    // Setup description.
    $description = t("You are about to delete assets from batch with title '%title'. !question This action can not be undone.", array('%title' => $batch['title'], '!question' => $question));

    $form['delete_assets'] = array(
      '#type' => 'fieldset',
      '#title' => t('Are you sure to delete the assets in the batch?'),
    );
    $form['delete_assets'] = array(
      '#markup' => '<b>' . $description . '</b>',
    );

    return $form;
  }

  /**
   * Process the asset.
   *
   * @param mediamosa_connector_response_asset $response
   *   The response from the asset call.
   * @param array $data
   *   Array of data that has been stored from the last process_asset() call.
   *   The $data array contains the submit of the possible options form
   *   submit.
   *
   * @return array
   *   Return the data array again to be stored for the next call.
   */
  public function process_asset(mediamosa_connector_response_asset $response, array $data = array()) {
    $user_id = $this->get_session_id();

    // Get the asset ID.
    $asset_id = $response->get_asset_id();

    // Delete asset.
    MediaMosaCkConnectorWrapper::delete_asset($asset_id, $user_id);

    return $data;
  }
}
