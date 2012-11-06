<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View will return the most popular assets.
 */

class mediamosa_ck_views_rest_asset_popular extends mediamosa_ck_views_rest_asset_search {

  protected function do_rest_call($view, $params, array $options = array()) {

    // If not set, use default for order;
    if (empty($this->orderby)) {
      $this->orderby[__CLASS__ . '.numofviews'] = array(
        'orderby' => 'viewed',
        'order' => 'DESC',
        'params' => array());
    }

    // Call parent.
    parent::do_rest_call($view, $params, $options);
  }

  /**
   * Returns the default value for mediamosa_is_public_list option.
   *
   * @return boolean
   *   Returns TRUE by default.
   */
  public function mediamosa_option_is_public_list_default() {
    return TRUE;
  }
}
