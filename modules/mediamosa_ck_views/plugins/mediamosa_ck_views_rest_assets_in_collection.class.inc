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
 * View will return the most popular assets.
 *
 */
class mediamosa_ck_views_rest_assets_in_collection extends mediamosa_ck_views_rest_asset_search {

  protected function do_rest_call($view, $params, array $options = array()) {

    // If not set, use default for order;
    if (empty($this->orderby)) {
      $this->orderby[__CLASS__ . '.' . 'numofviews'] = array('orderby' => 'videotimestamp', 'order' => 'ASC', 'params' => array());
    }

    if (empty($view->args)) {
      return;
    }

    // Expect first argument to be the collection ID.
    $options['cql'] = 'coll_id="^' . reset($view->args) . '^"';

    // Call parent.
    parent::do_rest_call($view, $params, $options);
  }
}
