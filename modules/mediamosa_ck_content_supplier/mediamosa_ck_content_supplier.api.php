<?php
/**
 * @file
 * API documentation MediaMosa CK content supplier.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Allows registration of a batch plugin function for batch processing.
 *
 * The callback in the batch is called during cron with one asset_id in the
 * batch. All callbacks are run serial, one by one, in order of creation of
 * queue. There can be multiple queues, but only one queue will run, running its
 * callbacks one by one. The callback is triggered by url and will run in its
 * own 'process' and will not stall the cron.
 *
 * @return array
 *   An associative array containing information about the batch function;
 *   - 'title'
 *     The translated title of the batch function, e.g. 'Generate still'.
 *   - 'description'
 *     The tranlated description of the batch.
 *   - 'class'
 *     Extend on class 'mediamosa_ck_content_supplier_queue' and use abstract
 *     functions.
 */
function hook_mediamosa_ck_content_supplier_plugin_info() {
  return array(
    'generate still' => array(
      'title' => t('Generate still'),
      'description' => t('Allows to generate still on the selection of the batch.'),
      'class' => 'mediamosa_ck_content_supplier_queue_generate_still',
    ),
  );
}

/**
 * Allows to alter the batch plugin info collection.
 *
 * @param array $batch_info
 *
 * @return array
 *   The altered batch info.
 */
function hook_mediamosa_ck_content_supplier_plugin_info_alter($batch_info) {
  if (isset($batch_info['generate still'])) {
    $batch_info['generate still']['settings_form'] = 'mediamosa_ck_cs_callback_generate_still_form_ext';
  }

  return $batch_info;
}
/**
 * @} End of "addtogroup hooks".
 */
