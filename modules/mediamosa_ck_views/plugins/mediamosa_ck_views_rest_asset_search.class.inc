<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 *
 * View is based on the REST call 'asset'.
 */

class mediamosa_ck_views_rest_asset_search extends mediamosa_ck_views_rest_asset {

  /**
   * Builds the necessary info to execute the query.
   *
   * @param view $view
   *   The view which is executed.
   */
  public function build(&$view) {
    // Call parent.
    parent::build($view);
  }

  /**
   * Execute the REST call.
   *
   * @param object $view
   *   The view object.
   * @param array $params
   *   Array of view options and settings.
   * @param array $options
   *   Options that can be used for overrides on the view params and REST call
   *   related options.
   */
  protected function do_rest_call($view, array $params, array $options = array()) {
    // Empty.
    $cql = array();
    if (isset($options['cql'])) {
      $cql[] = $options['cql'];
    }

    // Add possible filters to CQL search string.
    // @todo: filters need some preprocessing. see self::build().
    if (!empty($params['filters'])) {
      // Add the filters.
      foreach ($params['filters'] as $cql_expression) {
        if (trim($cql_expression) !== '') {
          $cql[] = trim($cql_expression);
        }
      }
    }

    // Any provided CQL? Add it at the end.
    if (isset($view->args['cql'])) {
      $cql[] = '(' . trim($view->args['cql'], '()') . ')';
    }

    // Glue with AND.
    $cql = implode(' AND ', $cql);

    $mm_options = array(
      'offset' => $params['start'],
      'limit' => $params['rows'],
      'hide_empty_assets' => $this->mediamosa_option_hide_empty_assets() ? 'TRUE' : 'FALSE',
      'is_public_list' => $this->mediamosa_option_is_public_list() ? 'TRUE' : 'FALSE',
      'show_stills' => $this->mediamosa_option_show_stills() ? 'TRUE' : 'FALSE',
      'show_collections' => $this->mediamosa_option_show_collections() ? 'TRUE' : 'FALSE',
      'cql' => $cql,
      'granted' => $this->mediamosa_option_granted() ? 'TRUE' : 'FALSE',
      'acl_user_id' => array(mediamosa_ck::session_user_id()),
    );

    // Mediamosa only supports one param order, so take the first.
    $orderby = empty($this->orderby) ? FALSE : reset($this->orderby);
    if ($orderby) {
      $mm_options['order_by'] = $orderby['orderby'];
      $mm_options['order_direction'] = $orderby['order'] == 'ASC' ? 'asc' : 'desc';
    }

    // Do the search.
    return MediaMosaCkConnectorWrapper::search_asset($mm_options);
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
    // Show query in admin view.
    $view->build_info['query'] = str_replace('&', "\n", $response->header->request_uri);

    $fields = array(
      'viewed', 'played',
      'videotimestamp', 'videotimestampmodified',
      'owner_id', 'group_id',
      'mediafile_duration',
    );

    // For pre-caching.
    $owners = array();

    // Now process the xml into an object views can work with.
    foreach ($response->xml->xpath('items/item') as $item) {
      $object = new stdClass();
      $object->asset_id = (string) $item->asset_id;
      $object->granted = MediaMosaCkConnectorWrapper::string2bool($item->granted) ? 'TRUE' : 'FALSE';
      $object->is_owner = mediamosa_ck::session_user_id() == $item->owner_id;

      // Other rights.
      $object->access_edit = $object->is_owner;
      $object->access_delete = $object->is_owner;

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
      $object->title = mediamosa_ck::get_asset_title($object);

      // Get the description value.
      $object->description = mediamosa_ck::get_asset_description($object);

      // Allow other modules to alter the object.
      drupal_alter('mediamosa_asset_view_object', $object);

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
   * Returns the value for mediamosa_is_public_list option.
   *
   * @return bool
   *   The value of mediamosa_is_public_list.
   */
  public function mediamosa_option_is_public_list() {

    if (isset($this->options['mediamosa_ck_views_is_public_list'])) {
      return $this->options['mediamosa_ck_views_is_public_list'];
    }

    // Return the public list option value.
    return TRUE;
  }

  /**
   * Returns the value for mediamosa_hide_empty_assets option.
   *
   * @return bool
   *   The value of mediamosa_is_public_list
   */
  public function mediamosa_option_hide_empty_assets() {

    if (isset($this->options['mediamosa_ck_views_hide_empty_assets'])) {
      return $this->options['mediamosa_ck_views_hide_empty_assets'];
    }

    // Hide by default.
    return TRUE;
  }

  /**
   * Returns the value for mediamosa_hide_empty_assets option.
   *
   * @return boolean
   *   The value of mediamosa_is_public_list
   */
  public function mediamosa_option_granted() {

    if (isset($this->options['mediamosa_ck_views_granted'])) {
      return $this->options['mediamosa_ck_views_granted'];
    }

    // Return the public list option value.
    return TRUE;
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['mediamosa_ck_views_is_public_list'] = array('default' => '');
    $options['mediamosa_ck_views_hide_empty_assets'] = array('default' => '');
    $options['mediamosa_ck_views_granted'] = array('default' => '');
    return $options;
  }

  /**
   * Provide a form to edit options for this plugin.
   */
  public function options_form(&$form, &$form_state) {
    // Call my parent.
    parent::options_form($form, $form_state);

    // Checkbox to allow show only public marked assets.
    $form['mediamosa_ck_views_is_public_list'] = array(
      '#title' => t('Only show public marked assets.'),
      '#description' => t("Assets marked with the 'is_public' flag will return in the view results when this option is checked. Unchecked will ignore the 'is_public' value on the asset."),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_is_public_list(),
    );

    // Checkbox to include empty assets.
    $form['mediamosa_ck_views_hide_empty_assets'] = array(
      '#title' => t('Hide assets that have no mediafiles.'),
      '#description' => t("Assets can be created without mediafiles or mediafile without content, therefor are not useful to include in most view results."),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_hide_empty_assets(),
    );

    // Checkbox to include empty assets.
    $form['mediamosa_ck_views_granted'] = array(
      '#title' => t('Filter on access.'),
      '#description' => t("By default, all media is included, even when its protected by access. You can use the value of 'granted' variable to see if the user has access to the media. When the value of granted is 'TRUE', then access is allowed. If granted is 'FALSE', then the user has no access to the media. This 'granted' value is used if you want to show the all the media, but use an 'lock' image to indicate that the user has no access. If you turn on the filter, it will only return media on which the user has access on. In that case the value of 'granted' is always 'TRUE'."),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_granted(),
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
   * @return array
   *   The views data.
   */
  public static function get_views_data($title, $class) {
    $data = array(
      'table' => array(
        'group' => t('MediaMosa'),
        'base' => array(
          'field' => 'id',
          'title' => $title,
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

      'metadata' => array(
        'title' => t('Search'),
        'help' => t('Search the assets with a pre configured metadata'),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_metadata',
        ),
      ),

      'metadata_selector' => array(
        'title' => t('Search metadata with selector'),
        'help' => t('Search the assets with metadata and a metadata selector'),
        'filter' => array(
          'handler' => 'mediamosa_ck_views_filter_metadataSelector',
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
