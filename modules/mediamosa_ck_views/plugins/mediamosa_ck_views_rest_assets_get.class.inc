<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View will return the featured assets.
 */

class mediamosa_ck_views_rest_assets_get extends mediamosa_ck_views_rest_asset_get {

  // ------------------------------------------------------------------ Methods.
  /**
   * Execute the REST call.
   *
   * @param object $view
   *   The view object.
   * @param array $params
   *   Array of view options and settings.
   * @param array $options
   *   Options that can be used for overrides on the view params and REST call
   *   related options.
   */
  protected function do_rest_call($view, array $params, array $options = array()) {

    // Get the asset_ids.
    $asset_ids = empty($view->args) ? array() : $view->args;

    // Any assets?
    if (empty($asset_ids)) {
      return;
    }

    // Must be array.
    if (!is_array($asset_ids)) {
      $asset_ids = array($asset_ids);
    }

    $mm_options = array(
      'offset' => $params['start'],
      'limit' => $params['rows'],
      'show_stills' => $this->mediamosa_option_show_stills() ? 'TRUE' : 'FALSE',
      'show_collections' => $this->mediamosa_option_show_collections() ? 'TRUE' : 'FALSE',
    );

    try {
      // Get the asset.
      return MediaMosaCkConnectorWrapper::get_assets($asset_ids, $mm_options);
    }
    catch (Exception $e) {
      mediamosa_ck::watchdog_error('Unable to retrieve asset from MediaMosa, message: @message.', array('@message' => $e->getMessage()));
    }
  }
}
