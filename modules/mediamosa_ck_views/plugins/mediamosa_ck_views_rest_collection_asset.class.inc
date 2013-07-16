<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View is based on the REST call 'asset'.
 */

class mediamosa_ck_views_rest_collection_asset extends mediamosa_ck_views_rest_asset_search {

  /**
   * Execute the REST call.
   *
   * @param object $view
   *   The view object.
   * @param array $params
   *   Array of options and settings.
   */
  protected function do_rest_call($view, array $params, array $options = array()) {
    // Get the collection ID.
    $coll_id = empty($view->args) ? '' : $view->args[0];

    if (empty($coll_id)) {
      return;
    }

    // Set the collection (unless already set).
    $options += array(
      'coll_id' => array($coll_id),
    );

    return parent::do_rest_call($view, $params, $options);
  }

  /**
   * Process the XML and fill the view data.
   *
   * @param mediamosa_connector_response $response
   *   The response object.
   * @param object $view
   *   The view object.
   * @param array $params
   *   Array of view options and settings.
   * @param array $options
   *   Options that can be used for overrides on the view params and REST call
   *   related options.
   */
  protected function process_rest_call(mediamosa_connector_response $response, $view, array $params, array $options = array()) {
    parent::process_rest_call($response, $view, $params, $options);
  }
}
