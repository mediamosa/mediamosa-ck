<?php
/**
 * @file
 * Media Management.
 */

class mediamosa_ck_media_management {
  // ------------------------------------------------------------------- Consts.
  // Permissions.
  const PERM_ACCESS_TOOLS_MEDIAMANAGEMENT = 'access media management';
  const PERM_ADMIN_MEDIA_MANAGEMENT = 'administer media management';

  // Max length title batch.
  const BATCH_MAX_TITLE_LENGTH = 128;
  const BATCH_MAX_DESCRIPTION_LENGTH = 1000;

  const DEFAULT_MAX_ITEMS = 15;


  /**
   * Matches all 'N' Unicode character classes (numbers)
   */
  public static $preg_class_numbers = '\x{30}-\x{39}\x{b2}\x{b3}\x{b9}\x{bc}-\x{be}\x{660}-\x{669}\x{6f0}-\x{6f9}\x{966}-\x{96f}\x{9e6}-\x{9ef}\x{9f4}-\x{9f9}\x{a66}-\x{a6f}\x{ae6}-\x{aef}\x{b66}-\x{b6f}\x{be7}-\x{bf2}\x{c66}-\x{c6f}\x{ce6}-\x{cef}\x{d66}-\x{d6f}\x{e50}-\x{e59}\x{ed0}-\x{ed9}\x{f20}-\x{f33}\x{1040}-\x{1049}\x{1369}-\x{137c}\x{16ee}-\x{16f0}\x{17e0}-\x{17e9}\x{17f0}-\x{17f9}\x{1810}-\x{1819}\x{1946}-\x{194f}\x{2070}\x{2074}-\x{2079}\x{2080}-\x{2089}\x{2153}-\x{2183}\x{2460}-\x{249b}\x{24ea}-\x{24ff}\x{2776}-\x{2793}\x{3007}\x{3021}-\x{3029}\x{3038}-\x{303a}\x{3192}-\x{3195}\x{3220}-\x{3229}\x{3251}-\x{325f}\x{3280}-\x{3289}\x{32b1}-\x{32bf}\x{ff10}-\x{ff19}';

  /**
   * Matches all 'P' Unicode character classes (punctuation)
   */
  public static $preg_class_punctuation = '\x{21}-\x{23}\x{25}-\x{2a}\x{2c}-\x{2f}\x{3a}\x{3b}\x{3f}\x{40}\x{5b}-\x{5d}\x{5f}\x{7b}\x{7d}\x{a1}\x{ab}\x{b7}\x{bb}\x{bf}\x{37e}\x{387}\x{55a}-\x{55f}\x{589}\x{58a}\x{5be}\x{5c0}\x{5c3}\x{5f3}\x{5f4}\x{60c}\x{60d}\x{61b}\x{61f}\x{66a}-\x{66d}\x{6d4}\x{700}-\x{70d}\x{964}\x{965}\x{970}\x{df4}\x{e4f}\x{e5a}\x{e5b}\x{f04}-\x{f12}\x{f3a}-\x{f3d}\x{f85}\x{104a}-\x{104f}\x{10fb}\x{1361}-\x{1368}\x{166d}\x{166e}\x{169b}\x{169c}\x{16eb}-\x{16ed}\x{1735}\x{1736}\x{17d4}-\x{17d6}\x{17d8}-\x{17da}\x{1800}-\x{180a}\x{1944}\x{1945}\x{2010}-\x{2027}\x{2030}-\x{2043}\x{2045}-\x{2051}\x{2053}\x{2054}\x{2057}\x{207d}\x{207e}\x{208d}\x{208e}\x{2329}\x{232a}\x{23b4}-\x{23b6}\x{2768}-\x{2775}\x{27e6}-\x{27eb}\x{2983}-\x{2998}\x{29d8}-\x{29db}\x{29fc}\x{29fd}\x{3001}-\x{3003}\x{3008}-\x{3011}\x{3014}-\x{301f}\x{3030}\x{303d}\x{30a0}\x{30fb}\x{fd3e}\x{fd3f}\x{fe30}-\x{fe52}\x{fe54}-\x{fe61}\x{fe63}\x{fe68}\x{fe6a}\x{fe6b}\x{ff01}-\x{ff03}\x{ff05}-\x{ff0a}\x{ff0c}-\x{ff0f}\x{ff1a}\x{ff1b}\x{ff1f}\x{ff20}\x{ff3b}-\x{ff3d}\x{ff3f}\x{ff5b}\x{ff5d}\x{ff5f}-\x{ff65}';

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
    return variable_get('mediamosa_ck_max_items_log_list', self::DEFAULT_MAX_ITEMS);
  }

  /**
   * Set the selection state.
   *
   * @param integer $uid
   *   The (Drupal) user ID.
   * @param string|array $asset_ids
   *   The asset ID(s).
   * @param boolean $state
   *   The on / off state. On will create entry, off will remove entry.
   */
  public static function set_selection_state($uid, $asset_ids, $state) {
    // Set state.
    if ($state) {
      if (is_array($asset_ids)) {
        foreach ($asset_ids as $asset_id) {
          self::set_selection_state($uid, $asset_id, $state);
        }
      }
      else {
        db_merge('mediamosa_ck_mm_selection')
          ->key(array(
            'uid' => $uid,
            'bid' => 0,
            'asset_id' => $asset_ids,
          ))
          ->fields(array(
            'uid' => $uid,
            'bid' => 0,
            'asset_id' => $asset_ids,
          ))
          ->execute();
      }
    }
    else {
      db_delete('mediamosa_ck_mm_selection')
        ->condition('uid', $uid)
        ->condition('bid', 0)
        ->condition('asset_id', $asset_ids)
        ->execute();
    }
  }

  /**
   * Get the selection count.
   *
   * @param integer $uid
   *   The (Drupal) user ID.
   */
  public static function get_selection_state_count($uid) {
    return db_select('mediamosa_ck_mm_selection', 's')
      ->condition('uid', $uid)
      ->condition('bid', 0)
      ->countQuery()
      ->execute()
      ->fetchField();
  }

  /**
   * Get the selection count text.
   *
   * @param integer $uid
   *   The (Drupal) user ID.
   */
  public static function get_selection_state_count_text() {
    global $user;
    $count = mediamosa_ck_media_management::get_selection_state_count($user->uid);
    return t('Selection (@count)', array('@count' => $count));
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

    return db_select('mediamosa_ck_mm_selection', 's')
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

    return db_select('mediamosa_ck_mm_selection', 's')
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
    return db_select('mediamosa_ck_mm_selection', 's')
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
    db_delete('mediamosa_ck_mm_selection')
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
      $bid = db_insert('mediamosa_ck_mm_batch')
        ->fields(array(
          'uid' => $user->uid,
          'created' => REQUEST_TIME,
          'title' => drupal_substr($title, 0, self::BATCH_MAX_TITLE_LENGTH),
          'description' => drupal_substr($description, 0, self::BATCH_MAX_DESCRIPTION_LENGTH),
        ))
        ->execute();

      // Move the current selection into the new batch.
      db_update('mediamosa_ck_mm_selection')
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

    $query = db_select('mediamosa_ck_mm_batch', 'b');
    $query->join('mediamosa_ck_mm_selection', 's', 's.bid = b.bid');
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
    $query = db_select('mediamosa_ck_mm_batch', 'b');
    $query->join('mediamosa_ck_mm_selection', 's', 's.bid = b.bid');
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
    return db_select('mediamosa_ck_mm_batch_queue', 'bq')
      ->condition('bid', $bid)
      ->condition(
        db_and()
          ->condition('state', mediamosa_ck_media_management_queue::STATE_RUNNING_START, '>=')
          ->condition('state', mediamosa_ck_media_management_queue::STATE_RUNNING_END, '<=')
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
      db_delete('mediamosa_ck_mm_batch_queue_log')
        ->condition('qid', db_select('mediamosa_ck_mm_batch_queue', 'q')->fields('q', array('qid')), 'IN')
        ->execute();

      db_delete('mediamosa_ck_mm_batch_queue')
        ->condition('bid', $bid)
        ->execute();

      // Delete the selection.
      db_delete('mediamosa_ck_mm_selection')
        ->condition('bid', $bid)
        ->execute();

      // Delete the batch.
      db_delete('mediamosa_ck_mm_batch')
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
    $plugins_infos = module_invoke_all('mediamosa_ck_media_management_plugin_info');

    // Allow other modules to alter the info.
    drupal_alter('mediamosa_ck_media_management_plugin_info', $plugins_infos);

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
    $values = empty($_SESSION['mediamosa_ck_mm']['search']['values']) ? array() : $_SESSION['mediamosa_ck_mm']['search']['values'];

    // Add defaults.
    $values += array(
      'search_operator' => 'and',
      'cql' => '',
      'search_collection' => 0,
      'search_published' => NULL,
      'search_text' => '',
    );

    if (!empty($_SESSION['mediamosa_ck_mm']['search']['search_metadata'])) {
      // Get metadata def.
      $metadata_definition = mediamosa_ck::get_metadata_fields();

      if (!is_array($_SESSION['mediamosa_ck_mm']['search']['search_metadata'])) {
        $_SESSION['mediamosa_ck_mm']['search']['search_metadata'] = array($_SESSION['mediamosa_ck_mm']['search']['search_metadata']);
      }

      foreach ($_SESSION['mediamosa_ck_mm']['search']['search_metadata'] as $prop => $items) {
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

    if ($values['search_text'] !== '') {
      $cql[] = '(' . self::search_text_to_cql($values['search_text']) . ')';
    }

    $cql_glue = ' ' . drupal_strtoupper($values['search_operator']) . ' ';
    if (!empty($values['cql'])) {
      $cql[] = '(' . $values['cql'] . ')';
    }

    switch ($values['search_published']) {
      case 'TRUE':
        $options['published'] = 'TRUE';
        break;

      case 'FALSE':
        $options['published'] = 'FALSE';
        break;

      case 'ALL':
      default:
        $options['published'] = 'ALL';
        break;
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

  /**
   * Implements hook_search_execute().
   *
   * Execute a search for a set of key words.
   *
   * Code partly based on parseSearchExpression() from search.extender.inc.
   *
   * @return string
   *   The CQL string.
   */
  public static function search_text_to_cql($search_text, $set_warnings = TRUE) {

    $keywords = array();

    // Matchs words optionally prefixed by a dash. A word in this case is
    // something between two spaces, optionally quoted.
    preg_match_all('/ (-?)("[^"]+"|[^" ]+)/i', ' ' .  $search_text, $keywords, PREG_SET_ORDER);

    if (count($keywords) == 0) {
      return;
    }

    $negatives = array();
    $positives = array();

    // Classify tokens.
    $or = FALSE;
    $warning = '';
    foreach ($keywords as $match) {
      $phrase = FALSE;
      // Strip off phrase quotes.
      if ($match[2]{0} === '"') {
        $match[2] = substr($match[2], 1, -1);
        $phrase = TRUE;
      }
      // Simplify keyword according to indexing rules and external
      // preprocessors. Use same process as during search indexing, so it
      // will match search index.
      $words = self::simplify($match[2]);
      // Re-explode in case simplification added more words, except when
      // matching a phrase.
      $words = $phrase ? array($words) : preg_split('/ /', $words, -1, PREG_SPLIT_NO_EMPTY);
      // Negative matches.
      if ($match[1] == '-') {
        $negatives = array_merge($negatives, $words);
      }
      // OR operator: instead of a single keyword, we store an array of all
      // OR'd keywords.
      elseif ($match[2] === 'OR' && count($positives)) {
        $last = array_pop($positives);
        // Starting a new OR?
        if (!is_array($last)) {
          $last = array($last);
        }
        $positives[] = $last;
        $or = TRUE;
        continue;
      }
      // AND operator: implied, so just ignore it.
      elseif ($match[2] == 'AND' || $match[2] == 'and') {
        $warning = $match[2];
        continue;
      }

      // Plain keyword.
      else {
        if ($match[2] === 'or') {
          $warning = $match[2];
        }
        if ($or) {
          // Add to last element (which is an array).
          $positives[count($positives) - 1] = array_merge($positives[count($positives) - 1], $words);
        }
        else {
          $positives = array_merge($positives, $words);
        }
      }
      $or = FALSE;
    }

    if ($warning === 'or' && $set_warnings) {
      drupal_set_message(t('Search for either of the two terms with uppercase <strong>OR</strong>. For example, <strong>cats OR dogs</strong>.'));
    }

    // Get the search fields.
    $search_fields = mediamosa_ck::get_metadata_search_fields();

    $cql = array();
    foreach ($positives as $positive) {
      // Group of or's.
      if (is_array($positive)) {
        $cql_positives = array();
        foreach ($positive as $positive_child) {
          foreach ($search_fields as $search_field) {
            $cql_positives[] = $search_field . '="' . addslashes($positive_child) . '"';
          }
        }
        $cql[] = count($cql_positives) > 1 ? '(' . implode(' OR ', $cql_positives) . ')' : implode('', $cql_positives);
      }
      else {
        $cql_positives = array();
        foreach ($search_fields as $search_field) {
          $cql_positives[] = $search_field . '="' . addslashes($positive) . '"';
        }
        $cql[] = count($cql_positives) > 1 ? '(' . implode(' OR ', $cql_positives) . ')' : implode('', $cql_positives);
      }
    }

    foreach ($negatives as $negative) {
      $cql_negatives = array();
      foreach ($search_fields as $search_field) {
        $cql_negatives[] = $search_field . ' NOT "' . addslashes($negative) . '"';
      }
      $cql[] = count($cql_negatives) > 1 ? '(' . implode(' OR ', $cql_negatives) . ')' : implode('', $cql_negatives);
    }

    // Rebuild into CQL.
    return implode(' AND ', $cql);
  }

  /**
   * Simplifies a string according to indexing rules.
   *
   * @param $text
   *   Text to simplify.
   *
   * @return
   *   Simplified text.
   *
   * @see hook_search_preprocess()
   */
  public static function simplify($text) {
    // Decode entities to UTF-8
    $text = decode_entities($text);

    // Lowercase
    $text = drupal_strtolower($text);

    // To improve searching for numerical data such as dates, IP addresses
    // or version numbers, we consider a group of numerical characters
    // separated only by punctuation characters to be one piece.
    // This also means that searching for e.g. '20/03/1984' also returns
    // results with '20-03-1984' in them.
    // Readable regexp: ([number]+)[punctuation]+(?=[number])
    $text = preg_replace('/([' . self::$preg_class_numbers . ']+)[' . self::$preg_class_punctuation . ']+(?=[' . self::$preg_class_numbers . '])/u', '\1', $text);

    // Multiple dot and dash groups are word boundaries and replaced with space.
    // No need to use the unicode modifer here because 0-127 ASCII characters
    // can't match higher UTF-8 characters as the leftmost bit of those are 1.
    $text = preg_replace('/[.-]{2,}/', ' ', $text);

    // The dot, underscore and dash are simply removed. This allows meaningful
    // search behavior with acronyms and URLs. See unicode note directly above.
    $text = preg_replace('/[._-]+/', '', $text);

    // With the exception of the rules above, we consider all punctuation,
    // marks, spacers, etc, to be a word boundary.
    $text = preg_replace('/[' . PREG_CLASS_UNICODE_WORD_BOUNDARY . ']+/u', ' ', $text);

    // Truncate everything to 50 characters.
    $words = explode(' ', $text);
    foreach ($words as &$word) {
      if (is_numeric($word)) {
        $word = ltrim($word, '0');
      }
      $word = truncate_utf8($word, 50);
    }

    return implode(' ', $words);
  }
}
