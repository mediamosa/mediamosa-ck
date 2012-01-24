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
 * View is based on the REST call 'asset'.
 *
 */
class mediamosa_ck_views_rest_asset_search extends mediamosa_ck_views_rest {

  /**
   * Execute the REST call.
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
      'hide_empty_assets' => $this->mediamosa_option_hide_empty_assets() ? 'TRUE' : 'FALSE',
      'is_public_list' => $this->mediamosa_option_is_public_list() ? 'TRUE' : 'FALSE',
      'cql' => $cql,
      'acl_user_id' => array(mediamosa_ck::session_user_id()),
    );

    // Mediamosa only supports one param order, so take the first.
    $orderby = empty($this->orderby) ? FALSE : reset($this->orderby);
    if ($orderby) {
      $options['order_by'] = $orderby['orderby'];
      $options['order_direction'] = $orderby['order'] == 'ASC' ? 'asc' : 'desc';
    }

    // Do the search.
    $response = MediaMosaCkConnectorWrapper::search_asset($options);

    $fields = array(
      'viewed', 'played',
      'videotimestamp', 'videotimestampmodified',
      'owner_id', 'group_id',
      'mediafile_duration',
    );

    // The name of the title/description field to use in metadata.
    $title_name = mediamosa_ck::get_metadata_title();
    $description_name = mediamosa_ck::get_metadata_description();

    // For pre-caching.
    $owners = array();

    // Now process the xml into an object views can work with.
    foreach($response->xml->xpath('items/item') as $item) {
      $object = new stdClass();
      $object->asset_id = (string) $item->asset_id;
      $object->granted = MediaMosaCkConnectorWrapper::string2bool($item->granted) ? 'TRUE' : 'FALSE';
      $object->is_owner = mediamosa_ck::session_user_id() == $item->owner_id;

      foreach (mediamosa_ck::get_metadata_fields() as $set => $metadata) {
        foreach ($item->xpath($metadata['xpath']) as $metadata_value) {
          foreach ($metadata_value as $name => $value) {
            $object->{$set . '.' . (string) $name} = (string) $value;
          }
        }
      }

      // Fill the array.
      foreach ($fields as $field) {
        // @todo need to get default value when value is empty.
        $object->{$field} = isset($item->{$field}) ? (string) $item->{$field} : '';
      }

      // Get the title value.
      $object->title = empty($object->{$title_name}) ? t('<no title>') : $object->{$title_name};

      // Get the description value.
      $object->description = empty($object->{$description_name}) ? t('<no description>') : $object->{$description_name};

      // Store the result.
      $view->result[] = $object;

      // Store the owner for later.
      $owners[] = (string) $item->owner_id;
    }

    // Precache the Drupal users so we can do quick lookup when showing owner.
    mediamosa_ck::precache_users_with_mails($owners);

    // Assign total number of records to get pager.
    $view->total_rows = (int) $response->xml->header->item_count_total;
  }

  /**
   * Returns the default value for mediamosa_is_public_list option.
   *
   * @return
   *   Returns FALSE by default.
   */
  public function mediamosa_option_is_public_list_default() {
    return FALSE;
  }

  /**
   * Returns the value for mediamosa_is_public_list option.
   *
   * @return
   *   The value of mediamosa_is_public_list
   */
  public function mediamosa_option_is_public_list() {

    if (isset($this->options['mediamosa_is_public_list'])) {
      return $this->options['mediamosa_is_public_list'];
    }

    // Return the public list option value.
    return $this->mediamosa_option_is_public_list_default();
  }

  /**
   * Returns the default value for mediamosa_hide_empty_assets option.
   *
   * @return
   *   Returns TRUE by default.
   */
  public function mediamosa_option_hide_empty_assets_default() {
    return TRUE;
  }

  /**
   * Returns the value for mediamosa_hide_empty_assets option.
   *
   * @return
   *   The value of mediamosa_is_public_list
   */
  public function mediamosa_option_hide_empty_assets() {

    if (isset($this->options['mediamosa_hide_empty_assets'])) {
      return $this->options['mediamosa_hide_empty_assets'];
    }

    // Return the public list option value.
    return $this->mediamosa_option_hide_empty_assets_default();
  }

  /**
   * Provide a form to edit options for this plugin.
   */
  public function options_form(&$form, &$form_state) {
    // Call my parent.
    parent::options_form($form, $form_state);

    // Checkbox to allow show only public marked assets.
    $form['mediamosa_is_public_list'] = array(
      '#title' => t('Only show public marked assets.'),
      '#description' => t("Assets marked with the 'is_public' flag will return in the view results when this option is checked. Unchecked will ignore the 'is_public' value on the asset."),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_is_public_list(),
    );

    // Checkbox to include empty assets.
    $form['mediamosa_hide_empty_assets'] = array(
      '#title' => t('Hide assets that have no mediafiles.'),
      '#description' => t("Assets can be created without mediafiles or mediafile without content, therefor are not useful to include in most view results."),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_hide_empty_assets(),
    );
  }

  /**
   * Validate the options form.
   */
  public function options_validate(&$form, &$form_state) {
    parent::options_validate($form, $form_state);
  }

  /**
   * Handle any special handling on the validate form.
   */
  public function options_submit(&$form, &$form_state) {
    parent::options_submit($form, $form_state);
  }

  /**
   * Is called when sort option is added/used.
   */
  public function add_orderby($table, $field = NULL, $order = 'ASC', $alias = '', $params = array()) {

    // Some fields have another orderby field...
    $translate = array(
      'viewed' => 'numofviews',
      'played' => 'numofplays',
    );

    // See if we have alias.
    if (isset($translate[$field])) {
      $field = $translate[$field];
    }

    parent::add_orderby($table, $field, $order, $alias, $params);
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
          'help' => t('Retrieve and search assets in the MediaMosa backend'),
          'query class' => $class,
        ),
      ),

      // Search fields.
      'keywords' => array(
        'title' => t('The search box'),
        'help' => t('Unique Identifier of each asset.'),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
      ),

      'coll_id' => array(
        'title' => t('Collection ID'),
        'help' => t('The collection ID of the object.'),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_collections',
        ),
        'argument' => array(
          'handler' => 'views_handler_argument_string',
        ),
      ),
    );

    // Add asset stuff.
    self::get_views_data_asset_common($data);

    // Owner / group.
    self::get_views_data_owner_stuff($data);

    // Add metadata.
    self::get_views_data_metadata($data);

    return $data;
  }
}
