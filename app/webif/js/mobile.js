/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

var currentSubscription = -1;
var currentArticle = -1;
var currentEnclosure = -1;
var currentDownload = -1;
var currentStatus = {};
var canFetchArticles = false;

var cutenews = new CuteNews();

function init() {
    document.getElementById("subscriptionsMenuButton").onclick = function () { showSubscriptionsMenu(); }
    document.getElementById("subscriptionsMenuBackground").onclick = function () { cancelSubscriptionsMenu(); }
    document.getElementById("updateSubscriptionsMenuItem").onclick = function () {
        updateAllSubscriptions();
        cancelSubscriptionsMenu();
    }
    
    document.getElementById("newSubscriptionMenuItem").onclick = function () {
        showNewSubscriptionTab("subscriptionsTab");
        cancelSubscriptionsMenu();
    }

    document.getElementById("downloadsMenuItem").onclick = function () {
        showDownloadsTab("subscriptionsTab");
        cancelSubscriptionsMenu();
    }

    document.getElementById("settingsMenuItem").onclick = function () {
        showSettingsTab("subscriptionsTab");
        cancelSubscriptionsMenu();
    }

    document.getElementById("articlesBackButton").onclick = function () { showSubscriptionsTab("articlesTab", true); }
    document.getElementById("articleBackButton").onclick = function () { showArticlesTab("articleTab", true); }
    document.getElementById("previousArticleButton").onclick = function () { previousArticle(); }
    document.getElementById("nextArticleButton").onclick = function () { nextArticle(); }
    document.getElementById("nextUnreadArticleButton").onclick = function () { nextUnreadArticle(); }
    document.getElementById("articleMenuButton").onclick = function () { showArticleMenu(); }
    document.getElementById("articleMenuBackground").onclick = function () { cancelArticleMenu(); }
    document.getElementById("openArticleMenuItem").onclick = function () {
        openArticleExternally(currentArticle);
        cancelArticleMenu();
    }
    
    document.getElementById("articleReadMenuItem").onclick = function () {
        toggleArticleRead(currentArticle);
        cancelArticleMenu();
    }
    
    document.getElementById("articleFavouriteMenuItem").onclick = function () {
        toggleArticleFavourite(currentArticle);
        cancelArticleMenu();
    }

    document.getElementById("enclosureMenuBackground").onclick = function () { cancelEnclosureMenu(); }
    document.getElementById("openEnclosureMenuItem").onclick = function () {
        openEnclosureExternally(currentEnclosure);
        cancelEnclosureMenu();
    }

    document.getElementById("downloadEnclosureMenuItem").onclick = function () {
        downloadEnclosure(currentEnclosure);
        cancelEnclosureMenu();
    }

    document.getElementById("downloadsBackButton").onclick = function () { showSubscriptionsTab("downloadsTab"); }
    document.getElementById("startDownloadsButton").onclick = function () { startDownloads(); }
    document.getElementById("pauseDownloadsButton").onclick = function () { pauseDownloads(); }
    document.getElementById("downloadMenuBackground").onclick = function () { cancelDownloadMenu(); }
    document.getElementById("cancelDownloadMenuItem").onclick = function () {
        cancelDownload(currentDownload);
        cancelDownloadMenu();
    }

    document.getElementById("cancelNewSubscriptionButton").onclick = function () { showSubscriptionsTab("newSubscriptionTab"); }
    document.getElementById("addSubscriptionButton").onclick = function () {
        addNewSubscription();
        showSubscriptionsTab("newSubscriptionTab");
    }

    document.getElementById("cancelSettingsButton").onclick = function () { showSubscriptionsTab("settingsTab"); }
    document.getElementById("saveSettingsButton").onclick = function () {
        saveSettings();
        showSubscriptionsTab("settingsTab");
    }    
    
    loadSubscriptions();
    checkStatus();
}

function showSubscriptionsTab(currentTab, positionAtCurrentSubscription) {
    if (currentTab) {
        document.getElementById(currentTab).setAttribute("data-current", "false");
    }
    
    document.getElementById("subscriptionsTab").setAttribute("data-current", "true");

    if ((positionAtCurrentSubscription) && (currentSubscription != -1)) {
        var offset = document.getElementById("subscriptionsList").childNodes[currentSubscription].offsetTop;
        
        if ((offset < window.pageYOffset) || (offset > window.pageYOffset + window.innerHeight)) {
            window.scrollTo(0, offset);
        }
    }

    window.onscroll = null;
}

function showArticlesTab(currentTab, positionAtCurrentArticle) {
    if (currentTab) {
        document.getElementById(currentTab).setAttribute("data-current", "false");
    }
    
    document.getElementById("articlesTab").setAttribute("data-current", "true");

    var list = document.getElementById("articlesList");
    
    if ((positionAtCurrentArticle) && (currentArticle != -1)) {
        var offset = list.childNodes[currentArticle].offsetTop;
        
        if ((offset < window.pageYOffset) || (offset > window.pageYOffset + window.innerHeight)) {
            window.scrollTo(0, offset);
        }
    }
    
    window.onscroll = function () {
        if ((window.pageYOffset + window.innerHeight == list.offsetTop + list.clientHeight) && (canFetchArticles)) {
            loadArticles(document.getElementById("subscriptionsList")
                         .childNodes[currentSubscription].getAttribute("data-id"),
                         list.childNodes.length, 50, false);
        }
    }   
}

function showArticleTab(currentTab) {
    if (currentTab) {
        document.getElementById(currentTab).setAttribute("data-current", "false");
    }
    
    document.getElementById("articleTab").setAttribute("data-current", "true");
    window.onscroll = null;
}

function showDownloadsTab(currentTab) {
    if (currentTab) {
        document.getElementById(currentTab).setAttribute("data-current", "false");
    }
    
    document.getElementById("downloadsTab").setAttribute("data-current", "true");
    window.onscroll = null;
    loadDownloads();
}

function showNewSubscriptionTab(currentTab) {
    if (currentTab) {
        document.getElementById(currentTab).setAttribute("data-current", "false");
    }
    
    document.getElementById("newSubscriptionTab").setAttribute("data-current", "true");
    document.getElementById("subscriptionTypeSelector").selectedIndex = 0;
    document.getElementById("subscriptionSourceField").value = "";
    document.getElementById("subscriptionEnclosuresCheckBox").checked = false;
    window.onscroll = null;
}

function showSettingsTab(currentTab) {
    if (currentTab) {
        document.getElementById(currentTab).setAttribute("data-current", "false");
    }
    
    document.getElementById("settingsTab").setAttribute("data-current", "true");
    window.onscroll = null;
    loadSettings();
}

function addNewSubscription() {
    cutenews.addSubscription(document.getElementById("subscriptionSourceField").value,
                             document.getElementById("subscriptionTypeSelector").selectedIndex,
                             document.getElementById("subscriptionEnclosuresCheckBox").checked);
}

function showSubscriptionsMenu() {
    document.getElementById("subscriptionsMenuBackground").style.display = "block";
    document.getElementById("subscriptionsMenu").style.display = "block";
}

function cancelSubscriptionsMenu() {
    document.getElementById("subscriptionsMenu").style.display = "none";
    document.getElementById("subscriptionsMenuBackground").style.display = "none";
}

function loadSubscriptions() {
    clearSubscriptions();
    cutenews.getSubscriptions(0, 0, function (subscriptions) {
        for (var i = 0; i < subscriptions.length; i++) {
            appendSubscription(subscriptions[i]);
        }
    }
    );
}

function appendSubscription(subscription) {
    insertSubscription(-1, subscription);
}

function insertSubscription(index, subscription) {    
    var list = document.getElementById("subscriptionsList");
    var count = list.childNodes.length;
    
    if ((index < 0) || (index >= count)) {
        index = count - 1;
    }
        
    var item = document.createElement("li");
    item.setAttribute("class", "ListItem");
    item.setAttribute("id", "subscription" + subscription.id);    
    item.setAttribute("title", subscription.title);
    item.setAttribute("data-current", "false");
    item.setAttribute("data-description", subscription.description);
    item.setAttribute("data-id", subscription.id);
    item.setAttribute("data-read", subscription.unreadArticles == 0 ? "true" : "false");
    item.setAttribute("data-source", subscription.source);
    item.setAttribute("data-sourcetype", subscription.sourceType);
    item.setAttribute("data-unreadarticles", subscription.unreadArticles);
    item.setAttribute("data-url", subscription.url);
    item.onclick = function () { setCurrentSubscription(index); showArticlesTab("subscriptionsTab"); };

    var content = document.createElement("div");
    content.setAttribute("class", "ListItemContent");

    var text = document.createElement("div");
    text.setAttribute("class", "ListItemText");
    text.innerHTML = subscription.title + "<br>" + formatDateTime(new Date(subscription.lastUpdated));
    content.appendChild(text);
    
    var bubble = document.createElement("div");
    bubble.setAttribute("class", "CountBubble");
    bubble.innerHTML = subscription.unreadArticles;
    content.appendChild(bubble);
    
    item.appendChild(content);
    
    list.insertBefore(item, list.childNodes[index]);
}

function removeSubscription(index) {    
    var list = document.getElementById("subscriptionsList");
    list.removeChild(list.childNodes[index]);
}

function clearSubscriptions() {
    currentSubscription = -1;
    
    var list = document.getElementById("subscriptionsList");
    
    for (var i = list.childNodes.length - 1; i > 0; i--) {
        list.removeChild(list.childNodes[i]);
    }
}

function setCurrentSubscription(index) {
    var list = document.getElementById("subscriptionsList");
    
    if (currentSubscription != -1) {
        list.childNodes[currentSubscription].setAttribute("data-current", "false");
    }
    
    currentSubscription = index;
    var item = list.childNodes[index];
    item.setAttribute("data-current", "true");
        
    if (list.scrollTop + list.clientHeight < item.offsetTop) {
        list.scrollTop = item.offsetTop - item.clientHeight - list.clientHeight;
    }
    else if (list.scrollTop > item.offsetTop) {
        list.scrollTop = item.offsetTop;
    }

    document.getElementById("articlesTitle").innerHTML = item.getAttribute("title");
    loadArticles(item.getAttribute("data-id"), 0, 50, true);
}

function updateSubscription(index) {
    if (currentStatus.status == 1) {
        return;
    }
    
    var id = document.getElementById("subscriptionsList").childNodes[index].getAttribute("data-id");
    cutenews.updateSubscription(id, checkStatus);
}

function updateAllSubscriptions() {
    if (currentStatus.status == 1) {
        return;
    }
    
    cutenews.updateSubscriptions(checkStatus);
}

function cancelSubscriptionUpdates() {
    if (currentStatus.status != 1) {
        return;
    }

    cutenews.cancelSubscriptionUpdates(checkStatus);
}

function checkStatus() {
    cutenews.getSubscriptionUpdateStatus(function (updateStatus) {
        if (updateStatus.status != currentStatus.status) {
            
            if (updateStatus.status == 1) {
                document.getElementById("updateSubscriptionsLabel").innerHTML = "Cancel updates";
                document.getElementById("updateSubscriptionsMenuItem").onclick = function () {
                    cancelSubscriptionUpdates();
                    cancelSubscriptionsMenu();
                }
            }
            else {
                document.getElementById("updateSubscriptionsLabel").innerHTML = "Update";
                document.getElementById("updateSubscriptionsMenuItem").onclick = function () {
                    updateAllSubscriptions();
                    cancelSubscriptionsMenu();
                }
            }
        }
        
        if (updateStatus.status == 1) {
            document.getElementById("subscriptionsTitle").innerHTML = updateStatus.statusText;
            window.setTimeout(checkStatus, 3000);
        }
        else {
            document.getElementById("subscriptionsTitle").innerHTML = "Subscriptions";
        }

        currentStatus = updateStatus;
    }
    );
}

function loadArticles(subscriptionId, offset, limit, clear) {
    if (clear) {
        clearArticles();
    }
    
    canFetchArticles = false;
    cutenews.getArticles(subscriptionId, offset, limit, function (articles) {
        for (var i = 0; i < articles.length; i++) {
            appendArticle(articles[i]);
        }
        
        canFetchArticles = true;
    }
    );
}

function appendArticle(article) {
    insertArticle(-1, article);
}

function insertArticle(index, article) {    
    var list = document.getElementById("articlesList");
    var count = list.childNodes.length;
    
    if ((index < 0) || (index >= count)) {
        index = count;
    }
    
    var item = document.createElement("li");
    item.setAttribute("class", "ListItem");
    item.setAttribute("id", "article" + article.id);
    item.setAttribute("title", article.title);
    item.setAttribute("data-author", article.author);
    item.setAttribute("data-body", article.body);
    item.setAttribute("data-categories", article.categories.join(", "));
    item.setAttribute("data-current", "false");
    item.setAttribute("data-enclosures", JSON.stringify(article.enclosures));
    item.setAttribute("data-favourite", article.favourite ? "true" : "false");
    item.setAttribute("data-id", article.id);
    item.setAttribute("data-read", article.read ? "true" : "false");
    item.setAttribute("data-subscriptionid", article.subscriptionId);
    item.setAttribute("data-url", article.url);
    item.onclick = function () { setCurrentArticle(index); showArticleTab("articlesTab"); }

    var content = document.createElement("div");
    content.setAttribute("class", "ListItemContent");

    var text = document.createElement("div");
    text.setAttribute("class", "ListItemText");
    text.innerHTML = article.title + "<br>" + formatDateTime(new Date(article.date));
    content.appendChild(text);
    
    item.appendChild(content);
    
    list.insertBefore(item, list.childNodes[index]);
}

function removeArticle(index) {    
    var list = document.getElementById("articlesList");
    list.removeChild(list.childNodes[index]);
}

function clearArticles() {
    currentArticle = -1;
    
    var list = document.getElementById("articlesList");
    
    for (var i = list.childNodes.length - 1; i >= 0; i--) {
        list.removeChild(list.childNodes[i]);
    }
}

function setCurrentArticle(index) {    
    var list = document.getElementById("articlesList");
    document.getElementById("previousArticleButton").disabled = (index == 0);
    document.getElementById("nextArticleButton").disabled = (index == list.childNodes.length - 1);
        
    if (currentArticle != -1) {
        list.childNodes[currentArticle].setAttribute("data-current", "false");
    }
    
    currentArticle = index;
    var item = list.childNodes[index];
    item.setAttribute("data-current", "true");
        
    if (list.scrollTop + list.clientHeight < item.offsetTop) {
        list.scrollTop = item.offsetTop - item.clientHeight - list.clientHeight;
    }
    else if (list.scrollTop > item.offsetTop) {
        list.scrollTop = item.offsetTop;
    }

    document.getElementById("articleTitle").innerHTML = item.getAttribute("title");
    document.getElementById("articleBody").innerHTML = "<b>Author:</b> "
        + item.getAttribute("data-author") + "<br><b>Categories:</b> " + item.getAttribute("data-categories")
        + "<br><br>" + item.getAttribute("data-body") + "<br><br>";
    document.getElementById("articleFavouriteLabel").innerHTML = (item.getAttribute("data-favourite") == "true" ? "Unfavourite" : "Favourite");

    
    if (item.getAttribute("data-read") == "true") {
        document.getElementById("articleReadLabel").innerHTML = "Mark as unread";
    }
    else {
        document.getElementById("articleReadLabel").innerHTML = "Mark as read";
        markArticleRead(index, true);
    }

    var enclosures = JSON.parse(item.getAttribute("data-enclosures"));
    
    if (enclosures.length > 0) {
        document.getElementById("articleEnclosures").style.display = "block";
        var subscriptionId = item.getAttribute("data-subscriptionid");
        var list = document.getElementById("enclosuresList");
        
        for (var i = list.childNodes.length - 1; i >= 0; i--) {
            list.removeChild(list.childNodes[i]);
        }
        
        for (var i = 0; i < enclosures.length; i++) {
            var ii = i;
            var url = enclosures[i].url;
            var type = enclosures[i].type;
            var size = enclosures[i].length;
            var item = document.createElement("li");
            item.setAttribute("class", "ListItem");
            item.setAttribute("data-subscriptionid", subscriptionId);
            item.setAttribute("data-url", url);
            item.onclick = function (event) { setCurrentEnclosure(ii); showEnclosureMenu(event); };

            var content = document.createElement("div");
            content.setAttribute("class", "ListItemContent");

            var text = document.createElement("div");
            text.setAttribute("class", "ListItemText");
            text.innerHTML = url + "<br>(" + type + ") " + formatBytes(size);
            content.appendChild(text);
            
            item.appendChild(content);

            list.appendChild(item);
        }
    }
    else {
        document.getElementById("articleEnclosures").style.display = "none";
    }
}

function previousArticle() {
    if (currentArticle > 0) {
        setCurrentArticle(currentArticle - 1);
    }
}

function nextArticle() {
    if (currentArticle < document.getElementById("articlesList").childNodes.length - 1) {
        setCurrentArticle(currentArticle + 1);
    }
}

function nextUnreadArticle() {
    var list = document.getElementById("articlesList");
    
    for (var i = Math.max(0, currentArticle); i < list.childNodes.length; i++) {
        if (list.childNodes[i].getAttribute("data-read") == "false") {
            setCurrentArticle(i);
            return;
        }
    }
    
    for (var i = 0; i < currentArticle; i++) {
        if (list.childNodes[i].getAttribute("data-read") == "false") {
            setCurrentArticle(i);
            return;
        }
    }    
}

function markArticleFavourite(index, favourite) {
    var article = document.getElementById("articlesList").childNodes[index];
    cutenews.markArticleFavourite(article.getAttribute("data-id"), favourite, function () {
        article.setAttribute("data-favourite", favourite);
        
        if (index == currentArticle) {
            document.getElementById("articleFavouriteLabel").innerHTML = (favourite ? "Unfavourite" : "Favourite");
        }
    }
    );
}

function toggleArticleFavourite(index) {
    markArticleFavourite(index, document.getElementById("articlesList").childNodes[index].getAttribute("data-favourite") == "false");
}

function markArticleRead(index, read) {
    var article = document.getElementById("articlesList").childNodes[index];
    cutenews.markArticleRead(article.getAttribute("data-id"), read, function () {
        article.setAttribute("data-read", read);
        
        if (index == currentArticle) {
            document.getElementById("articleReadLabel").innerHTML = "Mark as " + (read ? "unread" : "read");
        }
        
        var subscription = document.getElementById("subscriptionsList").childNodes[currentSubscription];
        var unread = Math.max(0, parseInt(subscription.getAttribute("data-unreadarticles")) + (read ? -1 : 1));
        subscription.setAttribute("data-unreadarticles", unread);
        subscription.childNodes[0].childNodes[1].innerHTML = unread;
        
        if (unread == 0) {
            subscription.setAttribute("data-read", "true");
            document.getElementById("nextUnreadArticleButton").disabled = true;
        }
        else {
            subscription.setAttribute("data-read", "false");
            document.getElementById("nextUnreadArticleButton").disabled = false;
        }
    }
    );
}

function toggleArticleRead(index) {
    markArticleRead(index, document.getElementById("articlesList").childNodes[index].getAttribute("data-read") == "false");
}

function openArticleExternally(index) {
    window.open(document.getElementById("articlesList").childNodes[index].getAttribute("data-url"));
}

function showArticleMenu() {
    document.getElementById("articleMenuBackground").style.display = "block";
    document.getElementById("articleMenu").style.display = "block";
}

function cancelArticleMenu() {
    document.getElementById("articleMenu").style.display = "none";
    document.getElementById("articleMenuBackground").style.display = "none";
}

function setCurrentEnclosure(index) {
    if (index == currentEnclosure) {
        return;
    }
    
    var list = document.getElementById("enclosuresList");
        
    if (currentEnclosure != -1) {
        list.childNodes[currentEnclosure].setAttribute("data-current", "false");
    }
    
    currentEnclosure = index;
    list.childNodes[index].setAttribute("data-current", "true");
}

function openEnclosureExternally(index) {
    window.open(document.getElementById("enclosuresList").childNodes[index].getAttribute("data-url"));
}

function downloadEnclosure(index) {
    var enclosure = document.getElementById("enclosuresList").childNodes[index];
    cutenews.addDownload(enclosure.getAttribute("data-url"), enclosure.getAttribute("data-subscriptionid"));
}

function showEnclosureMenu(event) {
    document.getElementById("enclosureMenuBackground").style.display = "block";
    var menu = document.getElementById("enclosureMenu");
    menu.style.display = "block";
    menu.style.top = Math.min(event.pageY, window.innerHeight - menu.clientHeight) + "px";
    menu.style.left = Math.min(event.pageX, window.innerWidth - menu.clientWidth) + "px";
}

function cancelEnclosureMenu() {
    document.getElementById("enclosureMenu").style.display = "none";
    document.getElementById("enclosureMenuBackground").style.display = "none";
}

function loadDownloads() {
    clearDownloads();
    cutenews.getDownloads(0, 0, function (downloads) {
        for (var i = 0; i < downloads.length; i++) {
            appendDownload(downloads[i]);
        }

        document.getElementById("startDownloadsButton").disabled =
            (document.getElementById("pauseDownloadsButton").disabled = (downloads.length == 0));
    }
    );
}

function appendDownload(download) {
    insertDownload(-1, download);
}

function insertDownload(index, download) {
    var list = document.getElementById("downloadsList");
    var count = list.childNodes.length;
    
    if ((index < 0) || (index >= count)) {
        index = count;
    }
    
    var item = document.createElement("li");
    item.setAttribute("class", "ListItem");
    item.setAttribute("id", "download" + download.id);
    item.setAttribute("title", download.fileName);
    item.setAttribute("data-current", "false");
    item.setAttribute("data-id", download.id);
    item.setAttribute("data-priority", download.priority);
    item.setAttribute("data-status", download.status);
    item.onclick = function (event) { setCurrentDownload(index); showDownloadMenu(event); }

    var content = document.createElement("div");
    content.setAttribute("class", "ListItemContent");

    var text = document.createElement("div");
    text.setAttribute("class", "ListItemText");
    text.innerHTML = download.fileName + "<br>" + download.statusString;
    content.appendChild(text);

    var progressBar = document.createElement("div");
    progressBar.setAttribute("class", "ProgressBar");
    var label = document.createElement("div");
    label.setAttribute("class", "ProgressBarLabel");
    label.innerHTML = formatBytes(download.bytesTransferred) + " of "
                      + formatBytes(download.size) + " (" + download.progress + "%)";
    var fill = document.createElement("div");
    fill.setAttribute("class", "ProgressBarFill");
    fill.style.width = download.progress + "%";
    progressBar.appendChild(label);
    progressBar.appendChild(fill);
    content.appendChild(progressBar);

    item.appendChild(content);
    
    list.insertBefore(item, list.childNodes[index]);
}

function removeDownload(index) {
    var list = document.getElementById("downloadsList");
    list.removeChild(list.childNodes[index]);
}

function clearDownloads() {
    currentDownload = -1;
    var list = document.getElementById("downloadsList");
    
    for (var i = list.childNodes.length - 1; i >= 0; i--) {
        list.removeChild(list.childNodes[i]);
    }
}

function setCurrentDownload(index) {
    if (index == currentDownload) {
        return;
    }
    
    var list = document.getElementById("downloadsList");
        
    if (currentDownload != -1) {
        list.childNodes[currentDownload].setAttribute("data-current", "false");
    }
    
    currentDownload = index;
    list.childNodes[index].setAttribute("data-current", "true");
}

function startDownloads() {
    cutenews.startDownloads(function () { loadDownloads(); });
}

function pauseDownloads() {
    cutenews.pauseDownloads(function () { loadDownloads(); });
}

function startDownload(index) {
    cutenews.startDownload(document.getElementById("downloadsList").childNodes[currentDownload].getAttribute("data-id"), function () { loadDownloads(); });
}

function pauseDownload(index) {
    cutenews.pauseDownload(document.getElementById("downloadsList").childNodes[currentDownload].getAttribute("data-id"), function () { loadDownloads(); });
}

function cancelDownload(index) {
    cutenews.cancelDownload(document.getElementById("downloadsList").childNodes[currentDownload].getAttribute("data-id"), function () { loadDownloads(); });
}

function showDownloadMenu(event) {
    var status = parseInt(document.getElementById("downloadsList").childNodes[currentDownload].getAttribute("data-status"));

    if (status > 4) {
        document.getElementById("startPauseDownloadLabel").innerHTML = "Pause";
        document.getElementById("startPauseDownloadMenuItem").onclick = function () {
            pauseDownload(currentDownload);
            cancelDownloadMenu();
        }
    }
    else {
        document.getElementById("startPauseDownloadLabel").innerHTML = "Start";
        document.getElementById("startPauseDownloadMenuItem").onclick = function () {
            startDownload(currentDownload);
            cancelDownloadMenu();
        }
    }
    
    document.getElementById("downloadMenuBackground").style.display = "block";
    var menu = document.getElementById("downloadMenu");
    menu.style.display = "block";
    menu.style.top = Math.min(event.pageY, window.innerHeight - menu.clientHeight) + "px";
    menu.style.left = Math.min(event.pageX, window.innerWidth - menu.clientWidth) + "px";
}

function cancelDownloadMenu() {
    document.getElementById("downloadMenu").style.display = "none";
    document.getElementById("downloadMenuBackground").style.display = "none";
}

function loadSettings() {
    cutenews.getSettings(function (settings) {
        document.getElementById("downloadPathField").value = settings.downloadPath;
        document.getElementById("concurrentDownloadsSelector").selectedIndex = settings.maximumConcurrentDownloads - 1;
        document.getElementById("automaticDownloadsCheckBox").checked = settings.startTransfersAutomatically;

        var selector = document.getElementById("networkProxyTypeSelector");
        document.getElementById("networkProxyCheckBox").checked = settings.networkProxyEnabled;
        document.getElementById("networkProxyHostField").value = settings.networkProxyHost;
        document.getElementById("networkProxyPortField").value = settings.networkProxyPort;
        document.getElementById("networkProxyUsernameField").value = settings.networkProxyUsername;
        document.getElementById("networkProxyPasswordField").value = settings.networkProxyPassword;

        for (var i = 0; i < selector.options.length; i++) {
            if (selector.options[i].value == settings.networkProxyType) {
                selector.selectedIndex = i;
                break;
            }
        }
    }
    );
}
        

function saveSettings() {
    var settings = {};
    settings["downloadPath"] = document.getElementById("downloadPathField").value;
    settings["maximumConcurrentTransfers"] = document.getElementById("concurrentDownloadsSelector").selectedIndex + 1;
    settings["startTransfersAutomatically"] = document.getElementById("automaticDownloadsCheckBox").checked;

    var selector = document.getElementById("networkProxyTypeSelector");
    settings["networkProxyEnabled"] = document.getElementById("networkProxyCheckBox").checked;
    settings["networkProxyType"] = selector.options[selector.selectedIndex].value;
    settings["networkProxyHost"] = document.getElementById("networkProxyHostField").value;
    settings["networkProxyPort"] = document.getElementById("networkProxyPortField").value;
    settings["networkProxyUsername"] = document.getElementById("networkProxyUsernameField").value;
    settings["networkProxyPassword"] = document.getElementById("networkProxyPasswordField").value;

    cutenews.setSettings(settings);
}
