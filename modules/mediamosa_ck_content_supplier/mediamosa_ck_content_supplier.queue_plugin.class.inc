<?php

abstract class mediamosa_ck_content_supplier_queue_plugin extends mediamosa_ck_content_supplier_queue {
  // ------------------------------------------------------------------- Consts.

  // --------------------------------------------------------------------- Vars.
  /**
   * The batch queue data.
   *
   * Array taken from mediamosa_ck_cs_batch_queue table.
   *
   * @var array
   */
  protected $batch_queue;

  /**
   * The parent batch data.
   *
   * Array taken from mediamosa_ck_cs_batch table.
   *
   * @var array
   */
  protected $batch;

  // ------------------------------------------------------- Methods (abstract).
  /**
   * Get the settings form.
   *
   * Return empty array or FALSE when the process has no settings.
   *
   * Use settings_form_validate($form, &$form_state) for hook_validate.
   * Use settings_form_submit($form, &$form_state) for hook_validate.
   *
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   * @param array $batch
   *   The batch data.
   *
   * @return array
   *   The settings form.
   */
  public static function settings_form($form, &$form_state, $batch) {
    return FALSE;
  }

  /**
   * Process the asset.
   *
   * @param mediamosa_connector_response_asset $response
   *   The response from the asset call.
   * @param array $data
   *   Array of data that has been stored from the last process_asset() call.
   *   The 'form_values' contains the submit of the possible options form
   *   submit.
   *
   * @return array
   *   Return the data array again to be stored for the next call.
   */
  abstract public function process_asset(mediamosa_connector_response_asset $response, array $data = array());

  // ------------------------------------------------------------------ Methods.
  /**
   * Implements hook_validate().
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   * @param array $batch
   *   The batch data.
   */
  public static function settings_form_validate($form, &$form_state, $batch) {
  }

  /**
   * Implements hook_submit().
   *
   * @param array $form
   *   The form array.
   * @param array $form_state
   *   An associative array containing the current state of the form.
   * @param array $batch
   *   The batch data.
   */
  public static function settings_form_submit($form, &$form_state, $batch) {
  }

  // -------------------------------------------------------------- Constructor.
  /**
   * The queue constructor.
   *
   * @param array $batch_queue
   *   The batch queue object.
   */
  function __construct(array $batch_queue) {
    // Get the batch queue array.
    $this->batch_queue = $batch_queue;
    if (empty($this->batch_queue)) {
      throw new Exception('Empty batch queue array');
    }

    // Load the batch data.
    $this->batch = self::get_batch($batch_queue['bid']);
    if (empty($this->batch)) {
      throw new Exception('Batch not found.');
    }
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Get the title of the batch.
   *
   * @return string
   *   The batch title.
   */
  public function get_title() {
    return $this->batch['title'];
  }

  /**
   * Process one run.
   */
  public function process() {
    $runs = 0;
    // Into ms.
    $max_run = ((self::LOCK_MAX_SECONDS - 2) * 1000);

    // Now set the timelimit to max seconds, just in case so we don't timeout.
    set_time_limit(self::LOCK_MAX_SECONDS);

    // So we know when we started.
    timer_start('cs_queue');

    // Starting.
    self::watchdog($this->batch_queue['qid'], 'Starting batch queue %name', array('%name' => $this->get_title()), WATCHDOG_INFO);

    // No longer than max seconds.
    while (timer_read('cs_queue') < $max_run) {

      // Update the state, might be canceled.
      $this->batch_queue['state'] = self::get_batch_queue_state($this->batch_queue['qid']);

      // Are we canceled?
      if ($this->batch_queue['state'] == self::STATE_RUNNING_CANCELING) {
        $this->batch_queue['state'] = self::set_batch_queue_state($this->batch_queue['qid'], self::STATE_DONE_CANCELED);
        break;
      }

      // Do next asset.
      if (!$this->process_next_asset()) {
        // Set state to done.
        $this->batch_queue['state'] = self::set_batch_queue_state($this->batch_queue['qid'], self::STATE_DONE);
        break;
      }
      $runs++;

      // If we pass 1/3 of the max time, then check if next run will be within
      // the max time.
      $time_passed = timer_read('cs_queue');
      if ($time_passed > ($max_run / 3)) {
        // Calculate if the next run will go over the max run.
        if (($time_passed / $runs) * ($runs + 1) >= $max_run) {
          break;
        }
      }
    }

    // Stop our timer.
    $timer = timer_stop('cs_queue');

    // Ending
    self::watchdog($this->batch_queue['qid'], 'Ending batch queue %name, total run time @seconds seconds, processed @assets assets.', array('%name' => $this->get_title(), '@seconds' => round($timer['time'] / 1000, 2), '@assets' => $runs), WATCHDOG_INFO);

    // Return TRUE when we are done.
    return self::is_state_done($this->batch_queue['state']);
  }

  /**
   * Process the next asset.
   *
   * Will put state on done when all assets are processed.
   *
   * @return boolean
   *   Return TRUE when asset was processed, FALSE when all assets are
   *   processed.
   */
  final public function process_next_asset() {
    // Get the current position.
    $position = $this->batch_queue['position']++;

    // Get the next asset ID to process.
    $asset_id = db_select('mediamosa_ck_cs_selection', 's')
      ->fields('s', array('asset_id'))
      ->condition('bid', $this->batch_queue['bid'])
      ->range($position, 1)
      ->execute()
      ->fetchField();

    // No more left?
    if (empty($asset_id)) {
      // Done.
      return FALSE;
    }

    try {
      // Get the asset array.
      $response = MediaMosaCkConnectorWrapper::get_asset($asset_id);

      if (!mediamosa_connector::check_result($response)) {
        self::watchdog($this->batch_queue['qid'], "Asset REST call returned code @code on asset (ID: @asset_id), messsage: '@message'.", array('@code' => $response->get_header_request_result_id(), '@asset_id' => $asset_id, '@message' => $response->get_header_request_result_description()), WATCHDOG_CRITICAL);
      }
      elseif ($response->get_header_item_count_total()) {
        // Add user id for use in processing of asset.
        $this->batch_queue['queue_plugin_data']['uid'] = $this->batch_queue['uid'];
        // Process asset.
        $this->batch_queue['queue_plugin_data'] = $this->process_asset($response, $this->batch_queue['queue_plugin_data']);
      }
      else {
        // No error, but no asset either?
        self::watchdog($this->batch_queue['qid'], "Asset REST call returned nothing on asset (ID: @asset_id), messsage: '@message'.", array('@asset_id' => $asset_id, '@message' => $response->get_header_request_result_description()), WATCHDOG_CRITICAL);
      }
    }
    catch (Exception $e) {
      self::watchdog($this->batch_queue['qid'], "Exception caught when processing asset (ID: @asset_id), exception messsage: '@message'.", array('@asset_id' => $asset_id, '@message' => $e->getMessage()), WATCHDOG_CRITICAL);
    }

    // Store the batch queue.
    mediamosa_ck_content_supplier_queue::store_batch_queue($this->batch_queue);

    // Done.
    return TRUE;
  }

  /**
   * Do post process.
   *
   * @param array $data
   *   Array of data that has been stored from the last process_asset() call.
   *   The $data array contains the submit of the possible options form
   *   submit.
   */
  public function post_process_assets(array $data = array()) {
  }
}
