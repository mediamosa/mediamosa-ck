<?php
/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl)
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
 * View is based on the REST call 'asset'.
 *
 */
class mediamosa_ck_views_rest_collection_asset extends mediamosa_ck_views_rest_asset_search {

  /**
   * Execute the REST call.
   *
   * @param $view
   *   The view object.
   * @param $params
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
