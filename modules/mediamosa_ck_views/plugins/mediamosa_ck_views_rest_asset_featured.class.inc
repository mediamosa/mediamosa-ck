<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View will return the featured assets.
 *
 */
class mediamosa_ck_views_rest_asset_featured extends mediamosa_ck_views_rest_asset_search {

  /**
   * Use this function to retrieve the current selected featured assets.
   *
   * @return
   *   Returns an array with current selected featured asset ids.
   */
  static public function get_featured_ids() {
    return variable_get('mediamosa_ck_featured_asset_ids', array());
  }

  /**
   * Use this function to set a featured id.
   *
   * @param $asset_id
   *   The ID of the asset to set as featured.
   */
  static public function set_featured_id($asset_id) {
    $asset_ids = self::get_featured_ids();

    // Lets limit the featured to 200 ids.
    while (count($asset_ids) > 200) {
      array_pop($asset_ids);
    }

    // Add to the asset ids.
    unset($asset_ids[$asset_id]);
    $asset_ids = array_merge(array($asset_id => $asset_id), $asset_ids);

    // Store.
    variable_set('mediamosa_ck_featured_asset_ids', $asset_ids);
  }

  static public function unset_featured_id($asset_id) {
    $asset_ids = self::get_featured_ids();

    if (!empty($asset_ids[$asset_id])) {
      unset($asset_ids[$asset_id]);

      // Store.
      variable_set('mediamosa_ck_featured_asset_ids', $asset_ids);
    }
  }

  /**
   * Executes the REST call and process the results.
   *
   * @param $view
   *   The view object.
   * @param $params
   *   Array of view options and settings.
   * @param $options
   *   Options that can be used for overrides on the view params and REST call
   *   related options.
   */
  protected function do_rest_call($view, $params, array $options = array()) {

    // If not set, use default for order;
    if (empty($this->orderby)) {
      $this->orderby[__CLASS__ . '.' . 'viewed'] = array('orderby' => 'videotimestamp', 'order' => 'DESC', 'params' => array());
    }

    // Featured assets are stored by their IDs in a global variable.
    $asset_ids = self::get_featured_ids();

    $cql = array();
    $limit = $params['rows'];
    $offset = $params['start'];

    foreach ($asset_ids as $asset_id) {
      if ($offset) {
        $offset--;
        continue;
      }

      // Add to CQL.
      $cql[] = 'asset_id == "^' . $asset_id . '^"';

      $limit--;
      if ($limit < 1) {
        break;
      }
    }

    // No asset_ids specified, then we just make one up so we get 0 results.
    if (empty($cql)) {
      $cql[] = 'asset_id == "^foo^"';
    }

    $options['cql'] = (!empty($options['cql']) ? '(' . trim($options['cql'], '()') . ') AND ' : '') . implode(' OR ', $cql);

    // Call parent.
    parent::do_rest_call($view, $params, $options);
  }

  /**
   * Returns the default value for mediamosa_is_public_list option.
   *
   * @return
   *   Returns TRUE by default.
   */
  public function mediamosa_option_is_public_list_default() {
    return TRUE;
  }
}
