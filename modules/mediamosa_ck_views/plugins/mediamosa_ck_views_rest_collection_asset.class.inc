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
  protected function do_rest_call($view, $params, array $options = array()) {
    // Get the asset_id.
    $coll_id = empty($view->args) ? '' : $view->args[0];

    if (empty($coll_id)) {
      return;
    }

    // Set the collection (unless already set).
    $options += array(
      'coll_id' => array($coll_id),
    );

    parent::do_rest_call($view, $params, $options);
  }
}
