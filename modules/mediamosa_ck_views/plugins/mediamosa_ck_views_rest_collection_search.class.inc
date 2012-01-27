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
 */
class mediamosa_ck_views_rest_collection_search extends mediamosa_ck_views_rest {

  /**
   * Execute the REST call.
   *
   * @param $view
   *   The view object.
   * @param $params
   *   Array of options and settings.
   */
  public function do_rest_call($view, $params, array $options = array()) {

    // Empty.
    $cql = array();

    // Add possible filters to CQL search string.
    if (!empty($params['filters'])) {
      // Add the filters.
      foreach ($params['filters'] as $cql_expression) {
        $cql[] = $cql_expression;
      }
    }

    // Any provided CQL? Add it at last.
    if (isset($view->args['cql'])) {
      $cql[] = '(' . trim($view->args['cql'], '()') . ')';
    }

    // Glue with AND.
    $cql = implode(' AND ', $cql);

    $options += array(
      'offset' => $params['start'],
      'limit' => $params['rows'],
      'cql' => $cql,
    );

    // Do the search.
    $xml = MediaMosaCkConnectorWrapper::search_collection($options);
    if (empty($xml)) {
      return;
    }

    // Get the fields.
    $fields = self::get_fields(self::get_views_data('foo', 'foo'));

    // For pre-caching.
    $owners = array();

    // Now process the xml into an object views can work with.
    foreach($xml->xpath('items/item') as $item) {
      $object = new stdClass();
      $object->is_owner = mediamosa_ck::session_user_id() == $item->owner_id;

      foreach ($fields as $field) {
        if (isset($item->{$field['name']})) {
          $object->{$field['name']} = (string) $item->{$field['name']};
        }
      }

      // Other rights.
      $object->access_edit = $object->is_owner;
      $object->access_delete = $object->is_owner;

      // Allow other modules to alter the object.
      drupal_alter('mediamosa_collection_view_object', $object);

      // Add to result.
      $view->result[] = $object;

      // Store the owner for later.
      $owners[] = (string) $item->owner_id;
    }

    // Precache the Drupal users so we can do quick lookup when showing owner.
    mediamosa_ck::precache_users_with_mails($owners);

    // Assign total number of records to get pager.
    $view->total_rows = (int) $xml->header->item_count_total;
  }

  /**
   * Get the views data for this REST call.
   *
   * @return
   */
  static public function get_views_data($title, $class) {
    $data = array(
      'table' => array(
        'group' => t('MediaMosa'),
        'base' => array(
          'field' => 'id',
          'title' => t($title),
          'help' => t('Search a collection using the MediaMosa backend'),
          'query class' => $class,
        ),
      ),
    );

    // Common.
    self::get_views_data_collection_common($data);

    // Owner / group.
    self::get_views_data_owner_stuff($data);

    return $data;
  }
}
