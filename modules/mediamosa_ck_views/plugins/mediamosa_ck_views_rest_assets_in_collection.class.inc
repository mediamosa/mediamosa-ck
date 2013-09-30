<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View will return the assets in the requested collection.
 */

class mediamosa_ck_views_rest_assets_in_collection extends mediamosa_ck_views_rest_asset_search {

  /**
   * Do rest call with order by timestamp.
   */
  protected function do_rest_call($view, array $params, array $options = array()) {

    // If not set, use default for order;
    if (empty($this->orderby)) {
      $this->orderby[__CLASS__ . '.numofviews'] = array(
        'orderby' => 'videotimestamp',
        'order' => 'ASC',
        'params' => array());
    }

    if (empty($view->args)) {
      return;
    }

    // Expect first argument to be the collection ID.
    $options['cql'] = 'coll_id=="^' . reset($view->args) . '^"';

    // Call parent.
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
