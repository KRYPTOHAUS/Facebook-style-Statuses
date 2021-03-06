<?php

/**
 * @file
 *   Preprocess functions for the Facebook-style Statuses theme templates.
 */
function template_preprocess_facebook_status_item(&$vars) {
  $status = $vars['status'];
  $options = $vars['options'];
  $context = facebook_status_determine_context($status->type);
  $vars['context'] = $context;
  $vars['sid'] = $status->sid;
  $vars['self'] = $status->type == 'user' && $status->sender == $status->recipient;
  $vars['page'] = $options['page'];
  $vars['type'] = $status->type;
  $vars['message'] = _facebook_status_run_filter($status->message);
  if (variable_get('facebook_status_nl2br', 0)) {
    $vars['message'] = nl2br($vars['message']);
  }
  $vars += array(
    'recipient' => '',
    'recipient_name' => '',
    'recipient_picture' => '',
    'meta' => '',
    'links' => '',
  );
  $vars['recipient'] = $context['handler']->load_recipient($status->recipient);
  $vars['recipient_name'] = check_plain($context['handler']->recipient_name($vars['recipient']));
  $vars['recipient_link'] = $context['handler']->recipient_link($vars['recipient']);
  if ($status->type == 'user') {
    $vars['recipient_picture'] = theme('user_picture', array('account' => $vars['recipient']));
  }
  $vars['sender'] = _facebook_status_user_load($status->sender);
  $vars['sender_name'] = check_plain($vars['sender']->name);
  $vars['sender_link'] = theme('username', array('account' => $vars['sender']));
  $vars['sender_picture'] = theme('user_picture', array('account' => $vars['sender']));
  if ($options['cacheable']) {
    $vars['created'] = format_date($status->created, 'short');
  }
  else {
    $vars['created'] = theme('facebook_status_time', array('time' => $status->created));
    if ($status->type == 'user') {
      if ($status->sender != $status->recipient || strpos($status->message, '@') === 0 || strpos($status->message, '[@') === 0) {
        $vars['meta'] = t('To @recipient', array('@recipient' => $vars['recipient_name']));
      }
    }
    elseif (!empty($vars['recipient_name'])) {
      $vars['meta'] = t('Posted on %entity', array('%entity' => $vars['recipient_name']));
    }
    if ($options['links']) {
      $vars['links'] = !empty($status->links) ? theme('links', array('links' => $node->links, 'attributes' => array('class' => 'links inline'))) : '';
    }
  }
}

/**
 * Returns themed HTML for a status.
 * @see facebook_status_show()
 */
function _facebook_status_show($status, $options) {
  if (!facebook_status_user_access('view', $status)) {
    return;
  }
  $options += array(
    'links' => TRUE,
    'page' => FALSE,
    'cacheable' => FALSE,
  );
  if ($links) {
    $status->links = module_invoke_all('link', 'facebook_status', $status);
    drupal_alter('facebook_status_link', $status->links, $status);
  }
  $message = trim($status->message);
  if ($options['page']) {
    $title = '';
    if (empty($message)) {
      drupal_not_found();
      return;
    }
    $sender = _facebook_status_user_load($status->sender);
    if ($status->type == 'user' && $status->sender == $status->recipient) {
      $title = t('@sender: @message', array('@sender' => $sender->name, '@message' => $message));
    }
    else {
      $recipient = _facebook_status_user_load($status->recipient);
      //"\xC2\xBB" is the unicode escape sequence for the HTML entity &raquo; (a double right angle bracket)
      $title = t("@sender \xC2\xBB @recipient: @message",
        array('@sender' => $sender->name, '@recipient' => $recipient->name, '@message' => $message)
      );
    }
    if (drupal_strlen($title) > 80) {
      //"\xE2\x80\xA6" is the unicode escape sequence for the HTML entity &hellip; (an ellipsis)
      $title = drupal_substr($title, 0, 79) . "\xE2\x80\xA6";
    }
    drupal_set_title($title);
  }
  drupal_add_css(drupal_get_path('module', 'facebook_status') . '/resources/facebook_status.css');
  return theme('facebook_status_item', array('status' => $status, 'options' => $options));
}
