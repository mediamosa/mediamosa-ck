<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View will return the most popular assets.
 */

class mediamosa_ck_views_rest_assets_in_collection extends mediamosa_ck_views_rest_asset_search {

  protected function do_rest_call($view, $params, array $options = array()) {

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
    parent::do_rest_call($view, $params, $options);
  }
}
