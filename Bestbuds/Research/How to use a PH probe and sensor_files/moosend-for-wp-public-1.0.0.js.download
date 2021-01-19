(function ($) {
    'use strict';

    Date.prototype.substractDays = function (days) {
        var dat = new Date(this.valueOf());
        dat.setDate(dat.getDate() - parseInt(days));
        return dat;
    }

    let urlExists = function (url, callback) {
        $.ajax({
            type: 'HEAD',
            url: url,
            success: function () {
                callback(true);
            },
            error: function () {
                callback(false);
            }
        });
    }

    let showModal = function (formId, modalParams, formsDisplayed) {
        setTimeout(function () {
            $("#" + formId).modal({
                fadeDuration: 300,
                fadeDelay: 0.5,
            });
        }, modalParams.popupDelay * 1000);
        formsDisplayed[formId] = new Date($.now());
        localStorage.setItem('formsDisplayed', JSON.stringify(formsDisplayed));
    }

    let getFormPromise = function (formId) {
        let data = {
            'action': 'get_form_details',
            'params': formId[formId.length - 1]
        };

        return Promise.resolve($.ajax({
            url: php_vars.ajax_url,
            type: "POST",
            data: data,
            dataType: 'json',
            async: false
        }));
    }

    let getFormObject = async function (formId) {
        try {
            let obj = await getFormPromise(formId);
            return obj;
        } catch (error) {
            console.log('Error:', error);
        }
        ;
    }

    let getClientIpAddressPromise = function () {
        let data = {
            'action': 'get_client_ip_address',
        };

        return Promise.resolve($.ajax({
            url: php_vars.ajax_url,
            type: "GET",
            data: data,
            dataType: 'json',
            async: false
        }));
    }

    let getClientIpAddress = async function () {
        try {
            let ip = await getClientIpAddressPromise();
            return ip;
        } catch (error) {
            console.log('Error:', error);
        }
        ;
    }

    let subscribeMember = function (params) {
        let data = {
            'action': 'subscribe_member',
            'params': params
        };

        return Promise.resolve($.ajax({
            url: php_vars.ajax_url,
            type: "POST",
            data: data,
            dataType: 'json',
            async: false
        }));
    }

    $(function () {
        let inputBorderStyle = $("#ms-sub-form").find('input').css('border');
        $("#ms-sub-form").find('.checkbox-style').css('border', inputBorderStyle);

        $('#TheCheckBox').css({
            '-webkit-appearance': ''
        });

        let clientIp;


        $('div[id^="ms4wp-"]').each(function () {

            let form = $(this);
            let formId = form.context.id;

            getFormObject(formId).then(function (formAtts) {
                form.submit(function (e) {
                    e.preventDefault();

                    getClientIpAddress().then(function (ip) {
                        let data = {
                            "MailingListId": formAtts.formList,
                            "MemberEmail": $(`#${formId} #email`).val(),
                            "Name": {
                                "Value": $(`#${formId} #name`).val()
                            },
                            "SubscribeType": 1,
                            "CustomFields": $(`#${formId} #custom-fields *`).serializeArray(),
                            "OriginalIpAddress": ip,
                            "SingleOptInStatus": {
                                "OptedInSource": 100
                            }
                        }

                        subscribeMember(data)
                            .then(function () {
                                if (formAtts.redirectUrl !== "") {
                                    if (formAtts.newTab === "true") {
                                        window.open(formAtts.redirectUrl, '_blank');
                                    } else {
                                        window.location.href = formAtts.redirectUrl;
                                    }
                                } else {
                                    $.modal.close();
                                }
                            })

                        return false;
                    });
                });

                let formsDisplayed = {};
                let pastDate = null;

                if (localStorage.getItem('formsDisplayed') !== null) {
                    formsDisplayed = JSON.parse(localStorage.getItem('formsDisplayed'));
                }

                if (formsDisplayed !== null && formId in formsDisplayed) {
                    pastDate = new Date(formsDisplayed[formId]);
                }

                let currentDate = new Date($.now());
                let dateCalc = (pastDate === null) ? true : currentDate.substractDays(formAtts.modalParams.popupFrequency) > new Date(pastDate);

                if (form.hasClass('popup-form') && dateCalc) {
                    if (formAtts.modalParams.exitIntent == "true") {
                        $(document).one("mouseleave", function () {
                            showModal(formId, formAtts.modalParams, formsDisplayed);
                        });
                    } else {
                        showModal(formId, formAtts.modalParams, formsDisplayed);
                    }
                }

                if (form.hasClass('basic')) {
                    $('#ms4wp-basic').removeAttr('disabled');
                } else if (form.hasClass('valign')) {
                    $('#ms4wp-valign').removeAttr('disabled');
                }
            });
        });
    });

})(jQuery);