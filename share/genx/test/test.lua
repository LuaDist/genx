require 'genx'

function testOutput(d)
    d:startElement('farm')

    d:startElement('crop')
    d:attribute('type', 'fruit')
    d:text('Apricots')
    d:endElement()

    d:startElement('crop')
    d:attribute('type', 'vegetable')
    d:text('Zucchini')
    d:endElement()

    d:endElement()
end

-- file output to stdout
doc = genx.new(io.stdout)
testOutput(doc)
print ''

-- sender output
doc = genx.new(io.write)
testOutput(doc)
print ''