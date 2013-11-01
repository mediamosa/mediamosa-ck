<?php
/**
 * @file
 * Delete assets in queue plugin.
 */

class mediamosa_ck_content_supplier_queue_plugin_collection_removefrom extends mediamosa_ck_content_supplier_queue_plugin {

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
    $collections = self::get_collections_for_options();

    $form['collections'] = array(
      '#type' => 'fieldset',
    );

    if (empty($collections)) {
      $form['collections']['select_collections']['#markup'] = t('No collections are found.');
    }
    else {
      $form['collections']['select_collections'] = array(
        '#type' => 'select',
        '#title' => t('Available collections'),
        '#description' => t('Select the collections.'),
        '#options' => $collections,
        '#size' => 8,
        '#multiple' => TRUE,
        '#required' => TRUE,
      );
    }

    return $form;
  }

  /**
   * Implements hook_validate().
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   * @param array $batch
   *   The batch data.
   */
  public static function settings_form_validate($form, &$form_state, $batch) {
    $collections = self::get_collections_for_options();

    if (empty($collections)) {
      form_set_error('select_collections', t('No collections are found.'));
    }
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

    foreach ($data['select_collections'] as $coll_id) {
      // Put asset in collections.
      MediaMosaCkConnectorWrapper::delete_collection_asset_relations($coll_id, $user_id, array($asset_id));
    }

    return $data;
  }

  /**
   * Returns the available collections for user.
   *
   * @return array
   *   The collections for options form item.
   */
  protected static function get_collections_for_options() {
    $user_id = mediamosa_ck::session_user_id();
    return MediaMosaCkConnectorWrapper::get_collections_for_options(array('cql' => 'owner_id == "^' . addslashes($user_id) . '^"'));
  }
}
