<?php
/**
 * @file
 * Content supplier.
 */

class mediamosa_ck_content_supplier {
  // ------------------------------------------------------------------- Consts.
  // Permissions.
  const PERM_ACCESS_CONTENT_SUPPLIER = 'access content supplier';
  const PERM_ADMIN_CONTENT_SUPPLIER = 'administer content supplier';

  // Max length title batch.
  const BATCH_MAX_TITLE_LENGTH = 128;
  const BATCH_MAX_DESCRIPTION_LENGTH = 1000;

  const DEFAULT_MAX_ITEMS = 25;
  // ---------------------------------------------------------------- Functions.
  /**
   * Get the setting for max items in the selection list.
   *
   * @return integer
   *   The max items.
   */
  public static function get_setting_max_items_search_result_list() {
    return variable_get('mediamosa_ck_max_items_search_result_list', self::DEFAULT_MAX_ITEMS);
  }

  /**
   * Get the setting for max items in the selection list.
   *
   * @return integer
   *   The max items.
   */
  public static function get_setting_max_items_process_selection_list() {
    return variable_get('mediamosa_ck_max_items_process_selection_list', self::DEFAULT_MAX_ITEMS);
  }

  /**
   * Get the setting for max items in the batch list.
   *
   * @return integer
   *   The max items.
   */
  public static function get_setting_max_items_batch_list() {
    return variable_get('mediamosa_ck_max_items_batch_list', self::DEFAULT_MAX_ITEMS);
  }

  /**
   * Get the setting for max items in the batch list.
   *
   * @return integer
   *   The max items.
   */
  public static function get_setting_max_items_queued_batches_list() {
    return variable_get('mediamosa_ck_max_items_queued_batches_list', self::DEFAULT_MAX_ITEMS);
  }

  /**
   * Get the setting for max items in the log listing.
   *
   * @return integer
   *   The max items.
   */
  public static function get_setting_max_items_log_list() {
    return variable_get('mediamosa_ck_max_items_log_list', 25);
  }

  /**
   * Set the selection state.
   *
   * @param integer $uid
   *   The (Drupal) user ID.
   * @param string $asset_id
   *   The asset ID.
   * @param boolean $state
   *   The on / off state. On will create entry, off will remove entry.
   */
  public static function set_selection_state($uid, $asset_id, $state) {
    // Set state.
    if ($state) {
      db_merge('mediamosa_ck_cs_selection')
        ->key(array(
          'uid' => $uid,
          'bid' => 0,
          'asset_id' => $asset_id,
        ))
        ->fields(array(
          'uid' => $uid,
          'bid' => 0,
          'asset_id' => $asset_id,
        ))
        ->execute();
    }
    else {
      db_delete('mediamosa_ck_cs_selection')
        ->condition('uid', $uid)
        ->condition('bid', 0)
        ->condition('asset_id', $asset_id)
        ->execute();
    }
  }

  /**
   * Get the selected asset_ids using an array IDs of possible selected assets.
   *
   * @param integer $uid
   *   The (Drupal) user ID.
   * @param array $asset_ids
   *   The asset ID to check.
   *
   * @return array
   *   Returns array with asset ids that are selected.
   */
  public static function get_selection_state($uid, array $asset_ids) {
    if (empty($asset_ids)) {
      return array();
    }

    return db_select('mediamosa_ck_cs_selection', 's')
      ->fields('s', array('asset_id'))
      ->condition('uid', $uid)
      ->condition('bid', 0)
      ->condition('asset_id', $asset_ids)
      ->execute()
      ->fetchCol();
  }

  /**
   * Get the total count of the assets in an batch.
   *
   * Use $bid = 0 for current selection.
   *
   * @param integer $bid
   *   The batch ID.
   *
   * @return integer
   *   The number of assets in the batch (or selection).
   */
  public static function get_batch_asset_count($bid = 0) {
    global $user;

    return db_select('mediamosa_ck_cs_selection', 's')
      ->condition('uid', $user->uid)
      ->condition('bid', $bid)
      ->countQuery()
      ->execute()
      ->fetchField();
  }

  /**
   * Return the asset ids in a batch.
   *
   * @param integer $uid
   *   The user ID.
   * @param integer $bid
   *   The batch ID, use 0 for current selection.
   * @param integer $offset
   *   The start position.
   * @param integer $limit
   *   The number of IDs.
   * @param string $orderby
   *   The sort order.
   *
   * @return array
   *   Returns array with asset ids.
   *
   */
  public static function get_batch_asset_ids($uid, $bid, $offset = 0, $limit = 25, $orderby = 'DESC') {
    return db_select('mediamosa_ck_cs_selection', 's')
      ->fields('s', array('asset_id'))
      ->condition('uid', $uid)
      ->condition('bid', $bid)
      ->range($offset, $limit)
      ->orderBy('sid', $orderby)
      ->execute()
      ->fetchCol();
  }

  /**
   *
   * @param integer $uid
   *   The user ID.
   * @param integer $bid
   *   The batch ID, use 0 for current selection.
   * @param integer $offset
   *   The start position.
   * @param integer $limit
   *   The number of IDs.
   * @param string $orderby
   *   The sort order.
   *
   */
  public static function get_batch_assets($uid, $bid, $offset = 0, $limit = 25) {
    // Collect the asset ID.
    $asset_ids = self::get_batch_asset_ids($uid, $bid, $offset, $limit);

    // Get them.
    return mediamosa_ck::get_assets_for_ui($asset_ids);
  }

  /**
   * Get assets for the selection listing.
   *
   * @param array $options
   *   An associative array;
   *   - 'offset'
   *     Start position.
   *   - 'limit'
   *     Maximum items.
   *
   * @return type
   */
  public static function get_selection_assets(array $options = array()) {
    global $user;

    $options += array(
      'offset' => 0,
      'limit' => self::get_setting_max_items_process_selection_list(),
    );

    // Collect the asset ID.
    return self::get_batch_assets($user->uid, 0, $options['offset'], $options['limit']);
  }

  /**
   * Delete the asset from the selection.
   *
   * @param integer $bid
   *   The batch ID.
   */
  public static function delete_batch_selection($bid) {
    // Delete the selection.
    db_delete('mediamosa_ck_cs_selection')
      ->condition('bid', $bid)
      ->execute();
  }

  /**
   * Create batch with current selection.
   *
   * @param string $title
   *   The title of the batch.
   * @param string $description
   *   Short description.
   *
   * @throws Exception
   */
  public static function create_batch_from_selection($title, $description) {
    global $user;

    if (!user_is_logged_in()) {
      mediamosa_ck::watchdog('Can not create batch; user must be logged in.', array(), WATCHDOG_CRITICAL);
      throw new Exception('Can not create batch; user must be logged in.');
    }

    $transaction = db_transaction();
    try {
      $bid = db_insert('mediamosa_ck_cs_batch')
        ->fields(array(
          'uid' => $user->uid,
          'created' => REQUEST_TIME,
          'title' => drupal_substr($title, 0, self::BATCH_MAX_TITLE_LENGTH),
          'description' => drupal_substr($description, 0, self::BATCH_MAX_DESCRIPTION_LENGTH),
        ))
        ->execute();

      // Move the current selection into the new batch.
      db_update('mediamosa_ck_cs_selection')
        ->fields(array(
          'bid' => $bid,
        ))
        ->condition('bid', 0)
        ->execute();
    }
    catch (Exception $e) {
      $transaction->rollback();
      throw $e;
    }
  }

  /**
   * Get batch info for the batch listing.
   *
   * @param array $options
   *   An associative array;
   *   - 'offset'
   *     Start position.
   *   - 'limit'
   *     Maximum items.
   *
   * @return type
   */
  public static function get_batches(array $options = array()) {
    global $user;

    $options += array(
      'offset' => 0,
      'limit' => self::get_setting_max_items_batch_list(),
    );

    $query = db_select('mediamosa_ck_cs_batch', 'b');
    $query->join('mediamosa_ck_cs_selection', 's', 's.bid = b.bid');
    $query->addExpression('COUNT(s.bid)', 'total');
    return $query
      ->fields('b')
      ->condition('b.uid', $user->uid)
      ->range($options['offset'], $options['limit'])
      ->orderBy('b.created')
      ->groupBy('s.bid')
      ->execute()
      ->fetchAllAssoc('bid', PDO::FETCH_ASSOC);
  }

  /**
   * Fetches a batch by its batch ID.
   *
   * @param $bid
   *   An integer representing the batch ID.
   *
   * @return
   *   A fully-loaded batch object if a role with the given ID exists, or FALSE
   *   otherwise.
   */
  public static function get_batch($bid) {
    $query = db_select('mediamosa_ck_cs_batch', 'b');
    $query->join('mediamosa_ck_cs_selection', 's', 's.bid = b.bid');
    $query->addExpression('COUNT(s.bid)', 'total');
    return $query
      ->fields('b')
      ->condition('b.bid', $bid)
      ->orderBy('b.created')
      ->groupBy('s.bid')
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Check if the batch can be deleted.
   *
   * If the batch is queued and running, the batch can not be deleted.
   *
   * @param integer $bid
   *   The batch ID.
   *
   * @return boolean
   *   Returns TRUE when batch can be deleted.
   */
  public static function can_delete_batch($bid) {
    return db_select('mediamosa_ck_cs_batch_queue', 'bq')
      ->condition('bid', $bid)
      ->condition(
        db_and()
          ->condition('state', mediamosa_ck_content_supplier_queue::STATE_RUNNING_START, '>=')
          ->condition('state', mediamosa_ck_content_supplier_queue::STATE_RUNNING_END, '<=')
      )
      ->countQuery()
      ->execute()
      ->fetchField() == 0;
  }

  /**
   * Create batch with current selection.
   *
   * @param string $title
   *   The title of the batch.
   * @param string $description
   *   Short description.
   *
   * @throws Exception
   */
  public static function delete_batch($bid) {
    if (!self::can_delete_batch($bid)) {
      throw new Exception('Batch can not be deleted.');
    }

    $transaction = db_transaction();
    try {
      db_delete('mediamosa_ck_cs_batch_queue_log')
        ->condition('qid', db_select('mediamosa_ck_cs_batch_queue', 'q')->fields('q', array('qid')), 'IN')
        ->execute();

      db_delete('mediamosa_ck_cs_batch_queue')
        ->condition('bid', $bid)
        ->execute();

      // Delete the selection.
      db_delete('mediamosa_ck_cs_selection')
        ->condition('bid', $bid)
        ->execute();

      // Delete the batch.
      db_delete('mediamosa_ck_cs_batch')
        ->condition('bid', $bid)
        ->execute();
    }
    catch (Exception $e) {
      $transaction->rollback();
      throw $e;
    }
  }

  /**
   * Collect the batch info data.
   *
   * @return array
   *   The batch info array.
   */
  public static function get_plugin_infos() {
    // Get all batch infos.
    $plugins_infos = module_invoke_all('mediamosa_ck_content_supplier_plugin_info');

    // Allow other modules to alter the info.
    drupal_alter('mediamosa_ck_content_supplier_plugin_info', $plugins_infos);

    // Done.
    return $plugins_infos;
  }

  /**
   * Get the options of dropdown list.
   *
   * @return array
   *   The options array to use for select.
   */
  public static function get_plugin_infos_options() {
    // Get the batches.
    $plugins_infos = self::get_plugin_infos();

    $options = array('' => t('Select batch operation'));
    if (!empty($plugins_infos)) {
      foreach ($plugins_infos as $name => $plugins_info) {
        $options[$name] = $plugins_info['title'];
      }
    }
    else {
      $options = array('' => t('No plugins found'));
    }

    return $options;
  }

  /**
   * Process the search session.
   */
  public static function search_assets(array $options = array()) {
    $cql = array();

    // Get the session values.
    $values = empty($_SESSION['mediamosa_ck_cs']['search']['values']) ? array() : $_SESSION['mediamosa_ck_cs']['search']['values'];

    // Add defaults.
    $values += array(
      'search_operator' => 'and',
      'cql' => '',
      'search_collection' => 0,
    );

    if (!empty($_SESSION['mediamosa_ck_cs']['search']['search_metadata'])) {
      // Get metadata def.
      $metadata_definition = mediamosa_ck::get_metadata_fields();

      if (!is_array($_SESSION['mediamosa_ck_cs']['search']['search_metadata'])) {
        $_SESSION['mediamosa_ck_cs']['search']['search_metadata'] = array($_SESSION['mediamosa_ck_cs']['search']['search_metadata']);
      }

      foreach ($_SESSION['mediamosa_ck_cs']['search']['search_metadata'] as $prop => $items) {
        // Prop is [set].[name]
        list($set, $name) = explode('.', $prop);
        foreach (array_keys($items) as $key_item) {
          // Build id.
          $id = $set . '-' . $name . '-' . $key_item;

          // Get possible value.
          $value = isset($values[$id]) && $values[$id] !== '' ? $values[$id] : NULL;
          $value_till = isset($values[$id . '-till']) && $values[$id . '-till'] !== '' ? $values[$id . '-till'] : NULL;

          if (isset($value)) {
            switch ($metadata_definition[$set]['fields'][$name]) {
              case 'DATETIME':
                $value = $value . drupal_substr('0000-01-01 00:00:00', drupal_strlen($value));

                if (isset($value_till)) {
                  $value_till = $value_till . drupal_substr('9999-12-31 23:59:59', drupal_strlen($value_till));
                }
                else {
                  $value_till = $value . drupal_substr('9999-12-31 23:59:59', drupal_strlen($value));
                }


                $cql[] = '(' . $prop . ' >= "' . $value . '" AND '  . $prop . ' <= "' . $value_till . '")';
                break;

              case 'INT':
              case 'CHAR':
                // Add to CQL.
                $cql[] = $prop . ' = "' . addslashes($value) . '"';
                break;
            }
          }
        }
      }
    }

    $cql_glue = ' ' . drupal_strtoupper($values['search_operator']) . ' ';
    if (!empty($values['cql'])) {
      $cql[] = '(' . $values['cql'] . ')';
    }

    // Build cql.
    $cql = implode($cql_glue, $cql);

    if (!empty($cql)) {
      $options['cql'] = $cql;
    }
    if (!empty($values['search_collection'])) {
      $options['coll_id'] = array($values['search_collection']);
    }

    // Search and return results.
    return mediamosa_ck::search_asset_for_ui($options);
  }
}
