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
abstract class mediamosa_ck_views_rest extends views_plugin_query {
  // ---------------------------------------------------------------- Variables.
  var $requests;
  var $aliases;
  var $params;
  var $orderby;
  var $required_fields;

  // ----------------------------------------------------- Functions (abstract).
  // Execute the rest call.
  abstract protected function do_rest_call($view, $params, array $options = array());
  // Return information for the views_data hook.
  abstract static public function get_views_data($title, $class);

  // ---------------------------------------------------------------- Functions.
  /**
   * Builds the necessary info to execute the query.
   *
   * @param $view
   *   The view object.
   */
  public function build($view) {
    // Setup the view object.
    $view->init_pager();

    // Let the pager modify the query to add limits.
    $this->pager->query();

    // Set aliases of the fields.
    foreach ($view->field as $field_name => &$field) {
      $field->field_alias = $field_name;
      $field->aliases['entity_type'] = 'entity_type';
    }

    // Add fields to the query so they will be shown in solr document.
    $this->params['fl'] = array_keys($view->field);
  }

  /**
   * Add a field to the query table, possibly with an alias. This will
   * automatically call ensure_table to make sure the required table
   * exists, *unless* $table is unset.
   *
   * @param $table
   *   The table this field is attached to, here it is the ID of the REST view
   *   class.
   * @param $field
   *   The name of the field to add.
   * @param $required
   *   TRUE or FALSE, will add the field to required listing as input.
   *
   * @return $name
   *   The name that this field can be referred to as. Usually this is the
   *   alias.
   */
  public function add_field($table, $field, $required = FALSE) {
    $alias = "{$table}_{$field}";
    $this->aliases[$table][$field] = $alias;

    if ($required) {
      $this->required_fields[$alias] = $alias;
    }

    return $alias;
  }

  /**
   * Is called when sort option is added/used.
   */
  public function add_orderby($table, $field = NULL, $order = 'ASC', $alias = '', $params = array()) {
    $this->orderby[$table . '.' . $field] = array('orderby' => $field, 'order' => $order, 'params' => $params);
  }

  /**
   * Executes the query and fills the associated view object with according
   * values.
   *
   * Values to set:
   * $view->result,
   * $view->total_rows,
   * $view->execute_time,
   * $view->pager['current_page'].
   */
  public function execute($view) {
    try {
      $start = microtime(TRUE);

      // Setup the params.
      $params = array();
      $params['rows'] = $this->pager->options['items_per_page'];
      $params['start'] = $this->pager->current_page * $this->pager->options['items_per_page'];

      // Params to copy.
      $param_fields = array(
        'q' => 'q',
        'fl' => 'fl',
        'filters' => 'filters',
        'sort' => 'sort',
      );

      // Copy needed fields.
      foreach ($param_fields as $param_field) {
        if (!empty($this->params[$param_field])) {
          $params[$param_field] = $this->params[$param_field];
        }
      }

      // Perform request to MediaMosa.
      $this->do_rest_call($view, $params);

      // Store the results.
      $this->pager->total_items = $view->total_rows;
      $this->pager->update_page_info();

      // We shouldn't use $results['performance']['complete'] here, since
      // extracting the results probably takes considerable time as well.
      $view->execute_time = microtime(TRUE) - $start;
    }
    catch (Exception $e) {
      $this->errors[] = $e->getMessage();
    }

    // @todo: unsure if we want to use drupal_set_message here.
    if (!empty($this->errors)) {
      foreach ($this->errors as $msg) {
        drupal_set_message($msg, 'error');
      }
      $view->result = array();
      $view->total_rows = 0;
      $view->execute_time = 0;
    }
  }

  public function add_filter($type, $value, $exclude = FALSE) {
    $this->params['filters'][$type] = ($exclude ? '-' : '') . $value;
  }

  public function add_filter_string($string) {
    $this->params['q.alt'][] = $string;
  }

  public function add_sort($field, $order) {
    $this->params['sort'][] = $field . ' ' . $order;
  }

  public function get_params() {
    return $this->params;
  }

  public function add_where($group, $field, $value = NULL, $operator = NULL) {
  }

  /**
   * Generates a unique placeholder used in the db query.
   */
  public function placeholder($base = 'views') {
    static $placeholders = array();
    if (!isset($placeholders[$base])) {
      $placeholders[$base] = 0;
      return ':' . $base;
    }
    else {
      return ':' . $base . ++$placeholders[$base];
    }
  }
  // -------------------------------------------------------- MediaMosa related.
  /**
   * Add metadata fields to the view data.
   *
   * @param $data
   *   The data array to add metadata definitions to.
   */
  static public function get_views_data_metadata(&$data) {
    // Add metadata.
    foreach (mediamosa_ck::get_metadata_fields() as $set => $metadata) {
      // Add the whole set as an block.
      $data[$set] = array(
        'title' => t($metadata['title']),
        'help' => t('Metadata block for set @set_label.', array('@set_label' => $metadata['title'])),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text_metadata',
        ),
      );

      foreach ($metadata['fields'] as $name => $type) {
        // 3 types; date, text and integer.
        switch ($type) {
          case 'DATETIME':
            $data[$set . '_' . $name] = array(
              'title' => t($set . '.' . $name),
              'help' => t('Metadata date field in @set_label.', array('@set_label' => $metadata['title'])),
              'field' => array(
                'handler' => 'mediamosa_ck_views_field_date',
                'click sortable' => TRUE,
              ),
              'sort' => array(
                'handler' => 'mediamosa_ck_views_sort',
              ),
            );
            break;

          case 'CHAR':
              default:
            $data[$set . '_' . $name] = array(
              'title' => t($set . '.' . $name),
              'help' => t('Metadata field in @set_label.', array('@set_label' => $metadata['title'])),
              'field' => array(
                'handler' => 'mediamosa_ck_views_field_text',
                'click sortable' => TRUE,
              ),
              'filter' => array(
                'handler' => 'mediamosa_ck_views_filter_text',
              ),
              'sort' => array(
                'handler' => 'mediamosa_ck_views_sort',
              ),
            );
            break;
          case 'INT':
            $data[$set . '_' . $name] = array(
              'title' => t($set . '.' . $name),
              'help' => t('Metadata field in @set_label.', array('@set_label' => $metadata['title'])),
              'field' => array(
                'handler' => 'mediamosa_ck_views_field_text',
                'click sortable' => TRUE,
              ),
//              'filter' => array(
//                'handler' => 'mediamosa_ck_views_filter_text',
//              ),
              'sort' => array(
                'handler' => 'mediamosa_ck_views_sort',
              ),
            );
            break;
        }
      }
    }
  }

  /**
   * Simple conversion from the data in get_views_data to fields in the output.
   *
   * For now this only works on data that is in the root of the output.
   */
  static public function get_fields($data) {
    $fields = array();

    foreach ($data as $field_name => $field) {
      // Include all that have a field handler.
      if (empty($field['field']['handler'])) {
        continue;
      }

      $fields[] = array('name' => $field_name);
    }

    return $fields;
  }

  /**
   * Add fields for owner and group.
   */
  static public function get_views_data_owner_stuff(&$data) {
    $data += array(
      'owner_id' => array(
        'title' => t('Owner'),
        'help' => t('The owner of the object.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_owner',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_owner',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'group_id' => array(
        'title' => t('Group'),
        'help' => t('The user group of the object.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
      ),

      'is_owner' => array(
        'title' => t('Is owner'),
        'help' => t('A boolean value to indicate ownership of the object. TRUE / Yes will allow the user to edit or delete the object.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_boolean',
        ),
      ),
    );
  }

  static public function get_views_data_collection_common(&$data) {
    $data += array(
      // Fields.
      'coll_id' => array(
        'title' => t('Collection ID'),
        'help' => t('Unique Identifier of each collection.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
          'click sortable' => TRUE,
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'title' => array(
        'title' => t('Title'),
        'help' => t('The title of the collection.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'description' => array(
        'title' => t('Description'),
        'help' => t('The description of the collection.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'private' => array(
        'title' => t('private'),
        'help' => t('Collection is marked private.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
      ),

      'public' => array(
        'title' => t('public'),
        'help' => t('Collection is marked public.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
      ),

      'category' => array(
        'title' => t('category'),
        'help' => t('Collection is marked for main category.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
      ),

      'is_unappropriate' => array(
        'title' => t('is_unappropriate'),
        'help' => t('Collection is marked for not appropriate.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
      ),

      'numofvideos' => array(
        'title' => t('numofvideos'),
        'help' => t('The number of videos in the collection.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_text',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'created' => array(
        'title' => t('Created'),
        'help' => t('The date when collection was created.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_date',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'changed' => array(
        'title' => t('Changed'),
        'help' => t('The date when collection was changed.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_date',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'actions' => array(
        'title' => t('Actions'),
        'help' => t('Allowed administration links.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_collection_actions',
        ),
      ),
    );
  }

  /**
   * Add common fields for asset.
   *
   * @param $data
   *   The data array to add asset definitions to.
   */
  static public function get_views_data_asset_common(&$data) {
    $data += array(
      // Fields.
      'asset_id' => array(
        'title' => t('Asset ID'),
        'help' => t('Unique Identifier of each asset.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
          'click sortable' => TRUE,
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'granted' => array(
        'title' => t('Granted'),
        'help' => t('Granted value indicates if you are allowed to see and play mediafile.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
        ),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_boolean',
        ),
      ),

      'viewed' => array(
        'title' => t('Viewed'),
        'help' => t('Number of times viewed.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
          'click sortable' => TRUE,
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'played' => array(
        'title' => t('Played'),
        'help' => t('Number of times played.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
          'click sortable' => TRUE,
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'videotimestamp' => array(
        'title' => t('Created'),
        'help' => t('When asset got media.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_date',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      'videotimestampmodified' => array(
        'title' => t('Changed'),
        'help' => t('When asset was modified.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_date',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),

      // Fields.
      'title' => array(
        'title' => t('Title'),
        'help' => t('The title of the asset. Is for display purpose only.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
          'click sortable' => FALSE,
        ),
      ),

      'description' => array(
        'title' => t('Description'),
        'help' => t('The description of the asset. Is for display purpose only.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_text',
          'click sortable' => FALSE,
        ),
      ),

      'still_url' => array(
        'title' => t('Still'),
        'help' => t('The thumbnail of the media.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_still',
        ),
      ),

      'player' => array(
        'title' => t('Video player'),
        'help' => t('The video (or media) player allows you to get either a play link (URL) or the HTML object code to include a player.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_player',
        ),
      ),

      'mediafile_duration' => array(
        'title' => t('Mediafile duration'),
        'help' => t('The duration of the original mediafile.'),
        'field' => array(
          'handler' => 'mediamosa_ck_views_field_mediafile_duration',
        ),
        'sort' => array(
          'handler' => 'mediamosa_ck_views_sort',
        ),
      ),
    );
  }

  /**
   * Fields that only are in the asset call.
   *
   * @param $data
   *   The data array to add asset definitions to.
   */
  static public function get_views_data_asset_specific(&$data) {
    $data += array(
    );
  }

  /**
   * Simple conversion from xmlobject to array.
   */
  static public function xmlobject2array($simplexmlelement) {
    $array = json_decode(json_encode($simplexmlelement), TRUE);
    self::_xmlobject2array_helper($array);
    return $array;
  }

  /**
   * Fixes empty array's.
   */
  static private function _xmlobject2array_helper(&$array) {
    foreach ($array as $key => &$child_array) {
      if (is_array($child_array)) {
        if (empty($child_array)) {
          $array[$key] = '';
        }
        else {
          self::_xmlobject2array_helper($child_array);
        }
      }
    }
  }

}
