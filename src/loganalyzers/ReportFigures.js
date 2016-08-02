var ReportFigures = new function() {

var TheFigures = {};
var TheLastId = 0;
var TheHelpDialog;
var TheShowHandlers = {};

this.init = function() {
	$('.ReportFigure').each(addControls);
	createHelp();
};

this.createInteractive = function(key, lines, data, options) {
	var cell = $('[key="' + key + '"] tr[type=interactive] td');
	var targetId = 'ReportFigure-' + TheLastId++;
	cell.find('.jqplot-target').attr('id', targetId);

	var textOptions = {
		fontSize: '12px',
		textColor: '#000000'
	};
	var plotOptions = {
		axesDefaults: {
			pad: 0,
			labelOptions: textOptions,
			tickOptions: textOptions
		},
		axes: {
			xaxis: { label: options['labelX1'] },
			yaxis: {
				label: options['labelY1'],
				labelRenderer: $.jqplot.CanvasAxisLabelRenderer
			},
			y2axis: {
				label: options['labelY2'],
				labelRenderer: $.jqplot.CanvasAxisLabelRenderer,
				tickOptions: { showGridline: false }
			}
		},
		cursor: {
			show: true,
			showTooltip: false,
			style: 'default',
			zoom: true
		},
		grid: {
			borderWidth: 0,
			shadow: false
		},
		seriesColors: [ '#0080ff', '#ff0000', '#00c000', '#c000ff' ],
		seriesDefaults: {
			lineWidth: 2,
			shadow: false,
			markerOptions: { shadow: false }
		},
		series: lines
	};

	jQuery.each(plotOptions['axes'], function (i, axis) {
		if (axis['label'].substr(axis['label'].length - 1) == '%') {
			axis['min'] = 0;
			axis['max'] = 100;
		}
	});
	plotOptions['axes']['y2axis']['tickOptions'] = { showGridline: false };

	var showLegend = jQuery.grep(lines, function(e) {
		return e['label'];
	}).length > 0;
	if (showLegend) {
		plotOptions['legend'] = {
			show: true,
			location: 's',
			placement: 'outsideGrid',
			renderer: $.jqplot.EnhancedLegendRenderer,
			rendererOptions: { numberColumns: 4 }
		};
	}

	if (options['dataStyle'] == 'points')
		plotOptions['seriesDefaults']['showLine'] = false;
	else
		plotOptions['seriesDefaults']['showMarker'] = false;

	jQuery.each([
		{ size: 6, style: 'plus' },
		{ size: 6, style: 'x' },
		{ size: 6, style: 'square' },
		{ size: 8, style: 'filledSquare' }
	], function (i, options) {
		var series = plotOptions['series'][i];
		if (series)
			series['markerOptions'] = options;
	});

	var plot = $.jqplot(targetId, data, plotOptions);

	var resizable = cell.find('.resizable');
	resizable.resizable({
		delay: 20,
		helper: 'ReportFigureResizableHelper',
		minWidth: resizable.outerWidth(),
		minHeight: resizable.outerHeight()
	}).bind('resizestop', function() {
		plot.replot({ resetAxes: true });
	});

	TheFigures[key] = plot;
};

var addControls = function(idx, fig) {
	var controls = $('<div class="controls HiddenWhenPrinted"></div>');
	$(fig).find('th div.header').append(controls);

	$(fig).find('tr[type]').each(function(i, tr) {
		var type = $(tr).attr('type');
		var link = $('<a type="' + type + '" href="#">' + type + '</a>')
			.click(change);
		controls.append(link).append(' &middot; ');
	});
	$('<a class="help" href="#">?</a>').click(showHelp).appendTo(controls);

	controls.find('a[type]:first').click();
};

var change = function(event) {
	event.preventDefault();

	var target = $(event.target);
	if (!target.attr('href'))
		return;

	var type = target.attr('type');
	var fig = target.parents('.ReportFigure');

	fig.find('.controls a[type][type!=' + type + ']').attr('href', '#');
	fig.find('tr[type][type!=' + type + ']').hide();

	fig.find('.controls a[type][type=' + type + ']').removeAttr('href');
	fig.find('tr[type][type=' + type + ']').show();
	if (TheShowHandlers[type])
		TheShowHandlers[type](fig, type);
};

var createHelp = function() {
	TheHelpDialog = $('<div class="ReportFigureHelp">' +

		'<p>There are two kind of figures: static and interactive. ' +
		'Static figures are regular images while interactive ' +
		'figures offer advanced features:</p>' +

		'<ul>' +
			'<li><b>Resizing</b>: drag bottom border, right ' +
				'border, or bottom right corner.</li>' +
			'<li><b>Zooming</b>: click and drag to select the ' +
				'area to zoom to, double click to reset ' +
				'zooming.</li>' +
			'<li><b>Line toggling</b>: click line title in the ' +
				'legend to toggle line visibility.</li>' +
		'</ul>' +

		'<p>By default, all figures are shown in static mode. You ' +
		'can switch between static and interactive figures using the ' +
		'links in the top right corner.</p>' +

		'<p>A modern browser is required for interactive figures.</p>' +

	'</div>').dialog({
		autoOpen: false,
		resizable: false,
		title: 'Interactive figures',
		width: '640px'
	});
	$('.ReportFigure th a.help').click(showHelp);
};

var showHelp = function() {
	if (TheHelpDialog.dialog('isOpen'))
		TheHelpDialog.dialog('option', 'position', 'center');
	else
		TheHelpDialog.dialog('open');
	return false;
}

TheShowHandlers['interactive'] = function(fig) {
	if (!TheFigures[fig.attr('key')]) {
		$.ajax({
			url: fig.find('tr[type=interactive]').attr('src'),
			dataType: 'script',
			crossDomain: true
		});
	}
};

};

$(document).ready(ReportFigures.init);
