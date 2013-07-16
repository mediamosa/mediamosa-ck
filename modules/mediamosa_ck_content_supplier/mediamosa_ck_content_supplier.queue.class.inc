<?php
/**
 * @file
 * All queue related functions.
 */

class mediamosa_ck_content_supplier_queue extends mediamosa_ck_content_supplier {
  // ------------------------------------------------------------------- Consts.
  // Lowest state #.
  const STATE_START = 0;

  // See get_state_text();
  const STATE_WAITING = 0;
  const STATE_WAITING_START = 0;
  const STATE_WAITING_END = 4;

  // Between 5 and 9 is running.
  const STATE_RUNNING = 5;
  const STATE_RUNNING_CANCELING = 6;
  const STATE_RUNNING_START = 5;
  const STATE_RUNNING_END = 9;

  // 10 or higher is done.
  const STATE_DONE = 10;
  const STATE_DONE_START = 10;
  const STATE_DONE_END = 99;

  // Queue was canceled during run.
  const STATE_DONE_CANCELED = 11;
  // Queue failed to run.
  const STATE_DONE_FAILED = 12;

  // Highest state #.
  const STATE_END = 99;

  // Lock name.
  const LOCK_NAME = 'mediamosa_ck_cs_queue';

  // Max time lock.
  const LOCK_MAX_SECONDS = 60;

  // ---------------------------------------------------------------- Functions.
  /**
   * Logs a system message.
   *
   * @param integer $qid
   *   The queue batch ID.
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   *   See t() for documentation on how $message and $variables interact.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param integer $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   */
  public static function watchdog($qid, $message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    $cs_queue_running = &drupal_static('cs_queue_running');
    $cs_queue_running = $qid;
    $variables['qid'] = $cs_queue_running;
    watchdog('Content supplier queue', $message, $variables, $severity);
  }

  /**
   * Get the batch queue data.
   *
   * @param integer $qid
   *   The queue ID.
   *
   * @return array
   *   The queue.
   *   - 'total'
   *     The number of log entries.
   */
  public static function get_batch_queue($qid) {
    $query = db_select('mediamosa_ck_cs_batch_queue', 'bq');
    $query->leftJoin('mediamosa_ck_cs_batch_queue_log', 'bql', 'bql.qid = bq.qid');
    $query->addExpression('COUNT(bql.qid)', 'total');
    $batch_queue = $query
      ->fields('bq')
      ->condition('bq.qid', $qid)
      ->orderBy('bq.created')
      ->groupBy('bql.qid')
      ->execute()
      ->fetchAssoc();

    if (!empty($batch_queue)) {
      // Unserialize the data blob.
      $batch_queue['queue_plugin_data'] = unserialize($batch_queue['queue_plugin_data']);
    }

    return $batch_queue;
  }

  /**
   * Get the batch ID from queue item.
   *
   * @param integer $qid
   *   The queue ID.
   *
   * @return integer
   *   The batch ID.
   */
  public static function get_batch_queue_bid($qid) {
    return db_select('mediamosa_ck_cs_batch_queue', 'bq')
      ->fields('bp', array('bid'))
      ->condition('qid', $qid)
      ->countQuery()
      ->execute()
      ->fetchField();
  }

  /**
   * Do one cron run.
   *
   * Should be called once every minute.
   */
  public static function do_cron() {
    // Although cron has its own locking, we'll do it too just in case we
    // are not called from hook_cron().
    $lock = lock_acquire(__CLASS__ . '::' . __FUNCTION__, 30.0);

    // Lock?
    if ($lock) {
      // Get the batch queue.
      $batch_queue = self::get_batch_for_running();

      // We got waiting batch.
      if (!empty($batch_queue)) {
        // Build url.
        $url = url(strtr('mediamosa/ck/cs/callback/@token/@qid', array('@token' => rawurlencode(mediamosa_ck::get_token()), '@qid' => rawurlencode($batch_queue['qid']))), array('absolute' => TRUE));

        // Trigger callback function.
        mediamosa_ck::trigger_http_request($url);
      }

      // Release lock.
      lock_release(__CLASS__ . '::' . __FUNCTION__);
    }
  }

  /**
   * Store the batch queue data.
   */
  public static function store_batch_queue($record) {
    if (empty($record['qid'])) {
      drupal_write_record('mediamosa_ck_cs_batch_queue', $record);
    }
    else {
      // Write.
      drupal_write_record('mediamosa_ck_cs_batch_queue', $record, 'qid');
    }
  }

  /**
   * Returns the number of batches in the queue.
   *
   * @param array $options
   *   - 'state_start'
   *     Select from this state and up.
   *   - 'state_end'
   *     Select from this state and down.
   *
   * @return integer
   *   The number of batches queued.
   *
   * @see self::get_queued_batches().
   */
  public static function count_queued_batches(array $options = array()) {
    unset($options['offset'], $options['limit']);
    return self::get_queued_batches($options, TRUE);
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
   *   - 'state_start'
   *     Select from this state and up.
   *   - 'state_end'
   *     Select from this state and down.
   * @param boolean $do_count_query
   *   Return the count on the query.
   *
   * @return array
   *   The queued batches.
   */
  public static function get_queued_batches(array $options = array(), $do_count_query = FALSE) {
    global $user;

    $options += array(
      'offset' => 0,
      'limit' => self::get_setting_max_items_queued_batches_list(),
     );

    $query = db_select('mediamosa_ck_cs_batch_queue', 'bq');
    $query->join('mediamosa_ck_cs_batch', 'b', 'b.bid = bq.bid');
    $query->join('mediamosa_ck_cs_selection', 's', 's.bid = bq.bid');
    if (isset($options['state_start']) || isset($options['state_end'])) {
      $options += array(
        'state_start' => self::STATE_START,
        'state_end' => self::STATE_END,
      );

      // Set range.
      list($state_start, $state_end) = array(min($options['state_start'], $options['state_end']), max($options['state_start'], $options['state_end']));

      $query
        ->condition(db_or()->condition(
          db_and()
            ->condition('bq.state', $state_start, '>=')
            ->condition('bq.state', $state_end, '<=')
        ));
    }

    $query
      ->condition('bq.uid', $user->uid)
      ->orderBy('bq.created')
      ->groupBy('bq.qid');

    // Return count result?
    if ($do_count_query) {
      return $query->countQuery()->execute()->fetchField();
    }

    $query
      ->range($options['offset'], $options['limit'])
      ->fields('bq')
      ->fields('b', array('title', 'description'));

    // For totals.
    $query->addExpression('COUNT(s.bid)', 'total');

    // Return normal array.
    return $query->execute()->fetchAllAssoc('qid', PDO::FETCH_ASSOC);
  }

  /**
   * Return the state text.
   *
   * @param integer $state
   *   The state to convert to text.
   * @param integer $position
   *   The position in the queue assets.
   * @param integer $total
   *   The total amount of assets.
   *
   * @return string
   *   The translated text.
   */
  public static function get_state_text($state, $position, $total) {
    $text = 'Unknown state.';

    // Calc percentage.
    $percentage = sprintf('%.02d', min($position, $total) / max(1, $total) * 100);

    switch ($state) {
      case self::STATE_WAITING:
        $text = 'waiting';
        break;

      case self::STATE_RUNNING:
        $text = 'running';
        break;

      case self::STATE_RUNNING_CANCELING:
        $text = 'running, canceling';
        break;

      case self::STATE_DONE:
        $text = 'done';
        break;

      case self::STATE_DONE_CANCELED:
        $text = 'canceled';
        break;

      case self::STATE_DONE_FAILED:
        $text = 'failed';
        break;
    }

    return t('At @percentage%, @state', array('@percentage' => $percentage, '@state' => $text));
  }

  /**
   * Is queue batch waiting?
   *
   * @param integer $state
   *   The state.
   *
   * @return boolean
   *   Returns TRUE when waiting, FALSE otherwise.
   */
  public static function is_state_waiting($state) {
    return ($state >= self::STATE_WAITING_START && $state <= self::STATE_WAITING_END);
  }

  /**
   * Is queue batch running?
   *
   * @param integer $state
   *   The state.
   *
   * @return boolean
   *   Returns TRUE when running, FALSE otherwise.
   */
  public static function is_state_running($state) {
    return ($state >= self::STATE_RUNNING_START && $state <= self::STATE_RUNNING_END);
  }

  /**
   * Is queue batch done?
   *
   * @param integer $state
   *   The state.
   *
   * @return boolean
   *   Returns TRUE when done, FALSE otherwise.
   */
  public static function is_state_done($state) {
    return ($state >= self::STATE_DONE_START && $state <= self::STATE_DONE_END);
  }

  /**
   * Get the first waiting or current running batch queue.
   *
   * @return array
   *   The batch queue array or FALSE when none are waiting/running.
   */
  public static function get_batch_for_running() {
    return db_select('mediamosa_ck_cs_batch_queue', 'bq')
      ->fields('bq')
      ->condition(
        db_or()
          ->condition(
            db_and()
              ->condition('state', self::STATE_WAITING_START, '>=')
              ->condition('state', self::STATE_WAITING_END, '<=')
          )
          ->condition(
            db_and()
              ->condition('state', self::STATE_RUNNING_START, '>=')
              ->condition('state', self::STATE_RUNNING_END, '<=')
          )
      )
      ->range(0, 1)
      ->orderBy('weight', 'ASC')
      ->orderBy('created', 'ASC')
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the state of the queue state.
   *
   * @param integer $qid
   *   The queue batch ID.
   *
   * @param integer
   *   The current state.
   */
  public static function get_batch_queue_state($qid) {
    return db_select('mediamosa_ck_cs_batch_queue', 'bq')
      ->fields('bq', array('state'))
      ->condition('qid', $qid)
      ->execute()
      ->fetchField();
  }

  /**
   * Set the state of the queue state.
   *
   * @param integer $qid
   *   The queue batch ID.
   * @param integer $state
   *   The new state.
   *
   * @return integer
   *   The state changed.
   */
  public static function set_batch_queue_state($qid, $state) {
    db_update('mediamosa_ck_cs_batch_queue')
      ->fields(array('state' => $state))
      ->condition('qid', $qid)
      ->execute();

    return $state;
  }

  /**
   * Cancel the queue batch.
   *
   * @param integer $qid
   *   The queue batch ID.
   *
   * @return integer
   *   The latest state.
   */
  public static function cancel($qid) {
    // Cancel batch queue.
    $state = self::get_batch_queue_state($this->batch_queue['qid']);

    // Done, or being canceled, leave.
    if (self::is_state_done($state) || $state == self::STATE_RUNNING_CANCELING) {
      // Already done or canceled.
      return $state;
    }

    self::watchdog($qid, 'Canceling batch.', array(), WATCHDOG_INFO);
    if (self::is_state_waiting($state)) {
      $state = self::STATE_DONE_CANCELED;
    }
    else {
      $state = self::STATE_RUNNING_CANCELING;
    }

    // Set batch queue state and return with it.
    return self::set_batch_queue_state($qid, $state);
  }

  /**
   * Run the queue.
   *
   * @param integer $qid
   *   The queue ID.
   */
  public static function run($qid) {
    // Store this, so watchdog dblog hook knows the qid.
    // @todo: write error handler.
    $cs_queue_running = &drupal_static('cs_queue_running', $qid);
    $cs_queue_running = $qid;

    // Only one process is allowed to run.
    if (!lock_acquire(self::LOCK_NAME, self::LOCK_MAX_SECONDS)) {
      self::watchdog($qid, 'Queue is already running', array(), WATCHDOG_INFO);
      return;
    }

    try {
      // Get queue.
      $batch_queue = self::get_batch_queue($qid);

      // Can not find queue?
      if (!$batch_queue) {
        throw new Exception('Queue object was not found!');
      }

      // When waiting state, set to running.
      if (self::is_state_waiting($batch_queue['state'])) {
        $batch_queue['state'] = self::set_batch_queue_state($qid, mediamosa_ck_content_supplier_queue::STATE_RUNNING);
      }

      // Create the batch queue object.
      $batch_queue_plugin = new $batch_queue['queue_plugin_class']($batch_queue);

      // Process one run. Returns TRUE when done.
      if (!$batch_queue_plugin->process()) {
        // Back to waiting state.
        $batch_queue['state'] = self::set_batch_queue_state($qid, mediamosa_ck_content_supplier_queue::STATE_WAITING);
      }
    }
    catch (Exception $e) {
      // Exceptions here are fatal.
      self::watchdog($qid, 'Exception caught, message; @message', array('@message' => $e->getMessage()), WATCHDOG_CRITICAL);

      // Set state to failed.
      self::set_batch_queue_state($qid, mediamosa_ck_content_supplier_queue::STATE_DONE_FAILED);
    }

    // Done.
    lock_release(self::LOCK_NAME);
    drupal_static_reset('cs_queue_running');
  }

  /**
   * Get the batch queue log.
   *
   * @param integer $qid
   *
   * @return array
   *   The log.
   */
  public static function get_batch_queue_log($qid, array $options = array()) {

    $options += array(
      'offset' => 0,
      'limit' => self::get_setting_max_items_log_list(),
    );

    return db_select('mediamosa_ck_cs_batch_queue_log', 'log')
      ->fields('log')
      ->condition('qid', $qid)
      ->range($options['offset'], $options['limit'])
      ->execute();
  }
}
