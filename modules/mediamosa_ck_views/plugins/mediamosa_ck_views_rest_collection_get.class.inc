<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 */

class mediamosa_ck_views_rest_collection_get extends mediamosa_ck_views_rest {

  /**
   * Execute the REST call.
   *
   * @param object $view
   *   The view object.
   * @param array $params
   *   unused.
   * @param array $options
   *   Options for REST call.
   */
  public function do_rest_call($view, array $params, array $options = array()) {
    // Total results.
    $view->total_rows = 0;

    // Get the asset_id.
    $coll_id = empty($view->args) ? '' : $view->args[0];

    if (empty($coll_id)) {
      return;
    }

    return MediaMosaCkConnectorWrapper::get_collection($coll_id, $options);
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
    // Get the fields.
    $fields = self::get_fields(self::get_views_data('foo', 'foo'));

    // Now process the xml into an object views can work with.
    foreach ($response->xpath('items/item') as $item) {
      $object = new stdClass();

      foreach ($fields as $field) {
        if (isset($item->{$field['name']})) {
          $object->{$field['name']} = (string) $item->{$field['name']};
        }
      }

      // Is owner of collection?
      $object->is_owner = mediamosa_ck::session_user_id() == $item->owner_id;

      // Other rights.
      $object->access_edit = $object->is_owner;
      $object->access_delete = $object->is_owner;

      // Allow other modules to alter the object.
      drupal_alter('mediamosa_collection_view_object', $object);

      // Add to result.
      $view->result[] = $object;
    }

    // Assign total number of records to get pager.
    $view->total_rows = (int) $xml->header->item_count_total;
  }

  /**
   * Get the views data for this REST call.
   *
   * @return array
   *  returns the views data.
   */
  public static function get_views_data($title, $class) {
    $data = array(
      'table' => array(
        'group' => t('MediaMosa'),
        'base' => array(
          'field' => 'id',
          'title' => t($title),
          'help' => t('Retrieve a collection using the MediaMosa backend'),
          'query class' => $class,
        ),
      ),
    );

    // Common.
    self::get_views_data_collection_common($data);

    // Owner / group.
    self::get_views_data_owner_stuff($data);

    // Make coll_id contextual.
    $data['coll_id']['argument'] = array(
      'handler' => 'views_handler_argument_string',
    );

    return $data;
  }
}
