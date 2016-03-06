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
var currentStatus = {};
var canFetchArticles = false;

var cutenews = new CuteNews();

function init() {
    loadSubscriptions();
    checkStatus();
    
    if (location.hash == "#downloads") {
        showDownloadsTab();
    }
    
    var list = document.getElementById("articlesList");
    list.onscroll = function () {
        if ((list.scrollTop == list.scrollHeight - list.clientHeight) && (canFetchArticles)) {
            loadArticles(document.getElementById("subscriptionsTable")
                         .childNodes[currentSubscription].getAttribute("data-id"),
                         list.childNodes.length, 10, false);
        }
    }
}

function showFeedsTab() {
    document.getElementById("downloadsTabButton").setAttribute("data-current", "false");
    document.getElementById("feedsTabButton").setAttribute("data-current", "true");
    document.getElementById("downloadsTab").style.display = "none";
    document.getElementById("feedsTab").style.display = "block";
}

function showDownloadsTab() {
    document.getElementById("feedsTabButton").setAttribute("data-current", "false");
    document.getElementById("downloadsTabButton").setAttribute("data-current", "true");
    document.getElementById("feedsTab").style.display = "none";
    document.getElementById("downloadsTab").style.display = "block";
    loadDownloads();
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
    var table = document.getElementById("subscriptionsTable");
    var count = table.childNodes.length;
    
    if ((index < 0) || (index >= count)) {
        index = count - 1;
    }
        
    var row = document.createElement("li");
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", "subscription" + subscription.id);    
    row.setAttribute("title", subscription.title);
    row.setAttribute("data-current", "false");
    row.setAttribute("data-description", subscription.description);
    row.setAttribute("data-id", subscription.id);
    row.setAttribute("data-read", subscription.unreadArticles == 0 ? "true" : "false");
    row.setAttribute("data-source", subscription.source);
    row.setAttribute("data-sourcetype", subscription.sourceType);
    row.setAttribute("data-unreadarticles", subscription.unreadArticles);
    row.setAttribute("data-url", subscription.url);
    row.onclick = function () { setCurrentSubscription(index); };
    
    var col = document.createElement("div");
    col.setAttribute("class", "SubscriptionIconColumn");
    
    var icon = document.createElement("img");
    icon.width = "16";
    icon.height = "16";
    icon.align = "left";
    icon.src = subscription.iconPath;
    col.appendChild(icon);
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "SubscriptionTitleColumn");
    col.innerHTML = subscription.title;
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "SubscriptionUnreadColumn");
    
    if (subscription.unreadArticles > 0) {
        col.innerHTML = subscription.unreadArticles;
    }
    
    row.appendChild(col);
    
    table.insertBefore(row, table.childNodes[index]);
}

function removeSubscription(index) {    
    var table = document.getElementById("subscriptionsTable");
    table.removeChild(table.childNodes[index]);
}

function clearSubscriptions() {
    currentSubscription = -1;
    
    var table = document.getElementById("subscriptionsTable");
    
    for (var i = table.childNodes.length - 1; i > 0; i--) {
        table.removeChild(table.childNodes[i]);
    }
}

function setCurrentSubscription(index) {
    var table = document.getElementById("subscriptionsTable");
    
    if (currentSubscription != -1) {
        table.childNodes[currentSubscription].setAttribute("data-current", "false");
    }
    
    currentSubscription = index;
    var row = table.childNodes[index];
    row.setAttribute("data-current", "true");
        
    if (table.scrollTop + table.clientHeight < row.offsetTop) {
        table.scrollTop = row.offsetTop - row.clientHeight - table.clientHeight;
    }
    else if (table.scrollTop > row.offsetTop) {
        table.scrollTop = row.offsetTop;
    }
    
    loadArticles(row.getAttribute("data-id"), 0, 10, true);
}

function updateCurrentSubscription() {
    if ((currentStatus.status == 1) || (currentSubscription == -1)) {
        return;
    }
    
    var id = document.getElementById("subscriptionsTable").childNodes[currentSubscription].getAttribute("data-id");
    cutenews.updateSubscription(id, checkStatus);
}

function updateAllSubscriptions() {
    if (currentStatus.status == 1) {
        return;
    }
    
    cutenews.updateSubscriptions(checkStatus);
}

function checkStatus() {
    cutenews.getSubscriptionUpdateStatus(function (updateStatus) {
        currentStatus = updateStatus;
        document.getElementById("statusBar").innerHTML = currentStatus.statusText;
        
        if (currentStatus.status == 1) {
            window.setTimeout(checkStatus, 3000);
        }
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
    
    var row = document.createElement("li");
    row.setAttribute("class", "Article");
    row.setAttribute("id", "article" + article.id);
    row.setAttribute("data-current", "false");
    row.setAttribute("data-id", article.id);
    
    var body = document.createElement("div");
    body.setAttribute("class", "ArticleBody");
    body.innerHTML = "<a target=\"_blank\" href=\"" + article.url + "\">" + article.title
    + "</a><br><br><b>Author:</b> " + article.author + "<br><b>Date:</b> " + formatDateTime(new Date(article.date))
    + "<br><b>Categories:</b> " + article.categories.join(", ") + "<br><br>" + article.body + "<br><br>";
    row.appendChild(body);
    row.appendChild(document.createTextNode("Favourite"));
    
    var checkbox = document.createElement("input");
    checkbox.setAttribute("type", "checkbox");
    checkbox.checked = article.favourite;
    checkbox.onclick = function () { cutenews.markArticleFavourite(article.id, checkbox.checked); }
    row.appendChild(checkbox);
    row.appendChild(document.createTextNode("Read"));
    
    checkbox = document.createElement("input");
    checkbox.setAttribute("type", "checkbox");
    checkbox.checked = article.read;
    checkbox.onclick = function () {
        var read = checkbox.checked;
        cutenews.markArticleRead(article.id, read,
            function () {
                var subscription = document.getElementById("subscriptionsTable").childNodes[currentSubscription];
                var unread = Math.max(0, parseInt(subscription.getAttribute("data-unreadarticles")) + (read ? -1 : 1));
                subscription.setAttribute("data-unreadarticles", unread);
                
                if (unread == 0) {
                    subscription.childNodes[2].innerHTML = "";
                    subscription.setAttribute("data-read", "true");
                }
                else {
                    subscription.childNodes[2].innerHTML = unread;
                    subscription.setAttribute("data-read", "false");
                }
            }
        );
    }
    row.appendChild(checkbox);
    
    if (article.enclosures.length > 0) {
        var header = document.createElement("h4");
        header.innerHTML = "Enclosures";
        row.appendChild(header);
        var table = document.createElement("ul");
        table.setAttribute("class", "TableView");
        
        for (var i = 0; i < article.enclosures.length; i++) {
            var url = article.enclosures[i].url;
            var type = article.enclosures[i].type;
            var size = article.enclosures[i].length;
            var enclosureRow = document.createElement("li");
            enclosureRow.setAttribute("class", "TableRow");
            var col = document.createElement("div");
            col.setAttribute("class", "EnclosureUrlColumn");
            var link = document.createElement("a");
            link.setAttribute("target", "_blank");
            link.setAttribute("href", url);
            link.innerHTML = url;
            col.appendChild(link);
            enclosureRow.appendChild(col);
            
            col = document.createElement("div");
            col.setAttribute("class", "EnclosureInfoColumn");
            col.innerHTML = formatBytes(size) + " (" + type + ")";
            enclosureRow.appendChild(col);
            
            col = document.createElement("div");
            col.setAttribute("class", "EnclosureActionsColumn");
            var button = document.createElement("input");
            button.setAttribute("class", "Button");
            button.setAttribute("type", "button");
            button.setAttribute("value", "Download");
            button.onclick = function () { cutenews.addDownload(url, subscriptionId); }
            col.appendChild(button);
            enclosureRow.appendChild(col);
            
            table.appendChild(enclosureRow);
        }
        
        row.appendChild(table);
    }
    
    list.insertBefore(row, list.childNodes[index]);
}

function removeArticle(index) {    
    var list = document.getElementById("articlesList");
    list.removeChild(list.childNodes[index]);
}

function clearArticles() {    
    var list = document.getElementById("articlesList");
    
    for (var i = list.childNodes.length - 1; i >= 0; i--) {
        list.removeChild(list.childNodes[i]);
    }
}

function loadDownloads() {
    clearDownloads();
    cutenews.getDownloads(0, 0, function (downloads) {
        for (var i = 0; i < downloads.length; i++) {
            appendDownload(downloads[i]);
        }
    }
    );
}

function appendDownload(download) {
    insertDownload(-1, download);
}

function insertDownload(index, download) {
    var table = document.getElementById("downloadsTable");
    var count = table.childNodes.length;
    
    if ((index < 0) || (index >= count)) {
        index = count;
    }
    
    var row = document.createElement("li");
    row.setAttribute("class", "TableRow");
    row.setAttribute("id", "download" + download.id);
    row.setAttribute("title", download.fileName);
    row.setAttribute("data-current", "false");
    row.setAttribute("data-id", download.id);
    row.setAttribute("data-priority", download.priority);
    row.setAttribute("data-status", download.status);
    row.onclick = function () { setCurrentDownload(index); }
    
    var col = document.createElement("div");
    col.setAttribute("class", "DownloadFileNameColumn");
    col.innerHTML = download.fileName;
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "DownloadPriorityColumn");
    var selector = document.createElement("select");
    selector.setAttribute("class", "Selector");
    var option = document.createElement("option");
    option.value = "HighPriority";
    option.selected = (download.priority == 0);
    option.appendChild(document.createTextNode("High"));
    selector.appendChild(option);
    option = document.createElement("option");
    option.value = "NormalPriority";
    option.selected = (download.priority == 1);
    option.appendChild(document.createTextNode("Normal"));
    selector.appendChild(option);
    option = document.createElement("option");
    option.value = "LowPriority";
    option.selected = (download.priority == 2);
    option.appendChild(document.createTextNode("Low"));
    selector.appendChild(option);
    selector.onchange = function () {
        cutenews.setDownloadProperties(download.id, {"priority": selector.options[selector.selectedIndex].value});
    }
    col.appendChild(selector);
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "DownloadProgressColumn");
    var progressBar = document.createElement("div");
    progressBar.setAttribute("class", "ProgressBar");
    var label = document.createElement("div");
    label.setAttribute("class", "ProgressBarLabel");
    label.appendChild(document.createTextNode(formatBytes(download.bytesTransferred) + " of "
                      + formatBytes(download.size) + " (" + download.progress + "%)"));
    var fill = document.createElement("div");
    fill.setAttribute("class", "ProgressBarFill");
    fill.style.width = download.progress + "%";
    progressBar.appendChild(label);
    progressBar.appendChild(fill);
    col.appendChild(progressBar);
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "DownloadStatusColumn");
    col.innerHTML = download.statusString;
    row.appendChild(col);
    
    col = document.createElement("div");
    col.setAttribute("class", "DownloadActionsColumn");
    var button = document.createElement("input");
    button.setAttribute("class", "Button");
    button.setAttribute("type", "button");
    button.setAttribute("value", download.status > 3 ? "Pause" : "Start");
    button.style.float = "left";
    button.onclick = function () {        
        if (parseInt(row.getAttribute("data-status")) > 3) {
            cutenews.pauseDownload(download.id, function () {
                loadDownloads();
            }
            );
        }
        else {
            cutenews.startDownload(download.id, function () {
                loadDownloads();
            }
            );
        }
    }
    col.appendChild(button);
    
    var cancelButton = document.createElement("input");
    cancelButton.setAttribute("type", "button");
    cancelButton.setAttribute("class", "Button");
    cancelButton.setAttribute("value", "Remove");
    cancelButton.style.float = "left";
    cancelButton.style.marginLeft = "4px";
    cancelButton.onclick = function () {
        cutenews.cancelDownload(download.id, function () {
            table.removeChild(row);
        }
        );
    }
    col.appendChild(cancelButton);
    row.appendChild(col);
    
    table.insertBefore(row, table.childNodes[index]);
}

function removeDownload(index) {
    var table = document.getElementById("downloadsTable");
    table.removeChild(table.childNodes[index]);
}

function clearDownloads() {
    currentDownload = -1;
    var table = document.getElementById("downloadsTable");
    
    for (var i = table.childNodes.length - 1; i >= 0; i--) {
        table.removeChild(table.childNodes[i]);
    }
}

function setCurrentDownload(index) {
    if (index == currentDownload) {
        return;
    }
    
    var table = document.getElementById("downloadsTable");
        
    if (currentDownload != -1) {
        table.childNodes[currentDownload].setAttribute("data-current", "false");
    }
    
    currentDownload = index;
    table.childNodes[index].setAttribute("data-current", "true");
}

function startDownloads() {
    cutenews.startDownloads(function () { loadDownloads(); });
}

function pauseDownloads() {
    cutenews.pauseDownloads(function () { loadDownloads(); });
}
